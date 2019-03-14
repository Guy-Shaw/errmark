/*
 * Filename: src/liberrmark/run-command.c
 * Project: errmark
 * Library: liberrmark
 * Brief: Run a given program; intercept and mark stderr
 *
 * Copyright (C) 2016-2019 Guy Shaw
 * Written by Guy Shaw <gshaw@acm.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cscript.h>     // for eprintf, fshow_wait_status, guard_malloc, debug
#include <errmark.h>     // for cmd_t, guard_ptrace, mark_close, after_write
#include <signal.h>      // for SIGCHLD, SIGTRAP
#include <stdbool.h>     // for false
#include <stddef.h>      // for NULL, size_t
#include <stdio.h>       // for fprintf, fwrite, stderr, perror, stdout
#include <stdlib.h>      // for exit
#include <sys/ptrace.h>  // for PTRACE_SETREGS, PTRACE_GETREGS, PTRACE_SYSCALL
#include <sys/user.h>    // for user_regs_struct
#include <sys/wait.h>    // for wait, WSTOPSIG, WIFSTOPPED
#include <syscall.h>     // for SYS_write
#include <unistd.h>      // for execvp, fork, sleep
#include <errmark.h>
#include <cscript.h>

/*
 * For __i386__ register definitions,
 * see:
 *   http://www.sco.com/developers/devspecs/abi386-4.pdf
 *
 *   https://uclibc.org/docs/psABI-i386.pdf
 *   Title: System V Application Binary Interface /
 *          Intel386 Architecture Processor Supplement
 *   Author: H. J. Lu          (Editor)
 *   Author: David L. Kreitzer (Editor)
 *   Author: Milind Girkar     (Editor)
 *   Author: Zia Ansari        (Editor)
 *   Date: 2015-02-03
 *   Version: 1.0
 *   Page ???  XXX No mention of system call portion of ABI
 *
 *
 *
 * For __x86_64__ register definitions,
 * see:
 *   http://www.x86-64.org/documentation/abi.pdf
 *   Title: System V Application Binary Interface /
 *          AMD64 Architecture Processor Supplement
 *   Author: Michael Matz   (Editor)
 *   Author: Jan Hubicka    (Editor)
 *   Author: Andreas Jaeger (Editor)
 *   Author: Mark Mitchell  (Editor)
 *   Date: 2013-10-07
 *   Version: Draft Version 0.99.6
 *   Page 123, A.2.1  Calling Conventions
 *
 *   find /usr/include -name 'reg.h' -print
 *       ==> /usr/include/x86_64-linux-gnu/sys/reg.h
 *
 *   find /usr/include -name 'user.h' -print
 *       ==> /usr/include/x86_64-linux-gnu/sys/user.h
 *
 *
 */

#if defined(__i386__)
#define reg_syscall orig_eax
#define reg_arg1    ebx
#define reg_arg2    ecx
#define reg_arg3    edx
#define reg_retn    eax
#elif defined(__x86_64__)
#define reg_syscall orig_rax
#define reg_arg1    rdi
#define reg_arg2    rsi
#define reg_arg3    rdx
#define reg_retn    rax
#else
#error "Need either __i386__  or  __x86_64__"
#endif

static int
ptrace_cmd(cmd_t *cmd)
{
    struct user_regs_struct regs;
    int exit_status = 0;
    int status;
    int toggle = 0;
    long ptrace_rc;

    int wfd;
    void *waddr;
    size_t wlen;

    while (1) {
        status = 0;
        wait(&status);
        if (status != 0 && !(WIFSTOPPED(status) && (WSTOPSIG(status) == SIGTRAP || WSTOPSIG(status) == SIGCHLD))) {
            exit_status = status;
            if (cmd->mark_state) {
                mark_close();
            }
            if (cmd->verbose) {
                eprintf("status=0x%02x\n", status);
            }
            break;
        }

        ptrace_rc = guard_ptrace(cmd, PTRACE_GETREGS, cmd->child, NULL, &regs);
        if (ptrace_rc == -1L && cmd->child_exited) {
            break;
        }

        if (regs.reg_syscall == SYS_write) {
            if (debug) {
                fprintf(stderr, "SYS_write; toggle=%d\n", toggle);
            }

            if (toggle == 0) {
                toggle = 1;
                wfd = (int)regs.reg_arg1;
                waddr = (void *)regs.reg_arg2;
                wlen = (size_t)regs.reg_arg3;

                if (debug) {
                    fprintf(stderr, "wfd  =%d\n",  wfd);
                    fprintf(stderr, "waddr=%p\n",  waddr);
                    fprintf(stderr, "wlen =%zu\n", wlen);
                }

#if 0
                if (wlen > 100) {
                    fprintf(stderr, "wlen forced to 100.\n");
                    wlen = 100;
                }
#endif

                if (wfd == 1 || wfd == 2) {
                    /*
                     * It is the start of a write system call,
                     * just before it will be performed by the kernel,
                     * and the destination fd is one we are interested in.
                     */
                    if (cmd->mark_state == 0) {
                        mark_open();
                        cmd->mark_state = 1;
                    }
                    before_write(wfd, waddr, wlen);
                    if (cmd->nullify) {
                        regs.reg_arg3 = 0;
                    }
                    guard_ptrace(cmd, PTRACE_SETREGS, cmd->child, NULL, &regs);
                    if (cmd->trace_fbt) {
                        fprintf(cmd->trace_fbt, "> write\n");
                    }

                    if (wfd == 2 && cmd->copy_fh != NULL) {
                        char *ebuf;

                        ebuf = (char *)guard_malloc(wlen);
                        pmem_copy(ebuf, cmd->child, waddr, wlen);
                        fwrite(ebuf, wlen, 1, stdout);
                        fwrite(ebuf, wlen, 1, cmd->copy_fh);
                    }
                    else {
                        pmem_fwrite(stdout, cmd->child, waddr, wlen);
                    }
                }
            }
            else {
                if (wfd == 1 || wfd == 2) {
                    after_write(wfd, waddr, wlen);
                    if (cmd->nullify) {
                        /*
                         * Since we have nullified the write(),
                         * we need to provide a fake return value
                         * of the original number of bytes to be written.
                         */
                        regs.reg_retn = wlen;
                    }
                }
                guard_ptrace(cmd, PTRACE_SETREGS, cmd->child, NULL, &regs);
                if (cmd->trace_fbt) {
                    fprintf(cmd->trace_fbt, "< write\n");
                }
                toggle = 0;
            }
            if (cmd->debug) {
                fprintf(stderr, ".\n");
                if (cmd->slow) {
                    sleep(1);
                }
            }
        }
        guard_ptrace(cmd, PTRACE_SYSCALL, cmd->child, NULL, NULL);
    }

    if (cmd->mark_state) {
        mark_close();
    }

    if (exit_status != 0) {
        if (cmd->verbose) {
            fshow_wait_status(stderr, cmd->cmd_name, exit_status);
        }
    }

    return (exit_status);
}

int
errmark_run_program(cmd_t *cmd)
{
    cmd->mark_state = 0;
    cmd->child_exited = false;
    cmd->child = fork();
    if (cmd->child == 0) {
        guard_ptrace(cmd, PTRACE_TRACEME, 0, NULL, NULL);
        cmd->rc = execvp(cmd->cmd_path, cmd->argv);
        if (cmd->rc == -1) {
            // XXX Use libexplain
            perror("execvp()");
        }
        else {
            eprintf("execvp() failed for reasons unknown!\n");
        }
        exit(2);
    }
    else {
        if (cmd->verbose) {
            eprintf("child pid=%d\n", cmd->child);
        }
        cmd->rc = ptrace_cmd(cmd);
        if (cmd->mark_state) {
            mark_close();
        }
        return (cmd->rc);
    }
}
