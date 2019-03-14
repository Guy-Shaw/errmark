/*
 * Filename: fshow-wait-status.c
 * Library: libcscript
 * Brief: Write an explanation of a wait() status to a given stdio stream
 *
 * Description:
 *   Given the status returned by one of the wait() family of system
 *   calls, write an explanation of that status to the given stdio stream.
 *   The status is broken out into signal and exit components.
 *   Any signal is shown both as a raw number and as a symbol,
 *   as defined in signal.h.
 *
 *   Special attention is paid to STOP and CONTINUE.
 *   If core was dumped, that is noted.
 *
 * Copyright (C) 2015-2016 Guy Shaw
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

#include <stdio.h>
#include <sys/wait.h>

extern char * decode_signal_r(char *buf, size_t sz, int sig);

/**
 * @brief Write an explanation of a wait() status to a given stdio stream
 *
 * @param f      IN  Write the decoded wait status to this FILE.
 * @param name   IN  The program name of the child process.
 * @param status IN  The exit status returned by a wait* system call.
 * @return void
 *
 */
void
fshow_wait_status(FILE *f, const char *name, int status)
{
    char sigbuf[32];
    int sig;

    fprintf(f, "%s ...\n", name);
    if (WIFEXITED(status)) {
        fprintf(f, "  exited, status=%d\n", WEXITSTATUS(status));
    }
    else if (WIFSIGNALED(status)) {
        sig = WTERMSIG(status);
        fprintf(f, "  killed by signal %d (%s)",
            sig, decode_signal_r(sigbuf, sizeof(sigbuf), sig));
#ifdef WCOREDUMP
        if (WCOREDUMP(status)) {
            fputs(" (core dumped)", f);
        }
#endif
        fputs("\n", f);
    }
    else if (WIFSTOPPED(status)) {
        sig = WSTOPSIG(status);
        fprintf(f, "  stopped by signal %d (%s)\n",
            sig, decode_signal_r(sigbuf, sizeof(sigbuf), sig));
    }
#ifdef WIFCONTINUED
    else if (WIFCONTINUED(status)) {
        fputs("  continued\n", f);
    }
#endif
    else {
        fprintf(f, "  *** INTERNAL ERROR *** status=x%02x\n",
            (unsigned int) status);
    }
}
