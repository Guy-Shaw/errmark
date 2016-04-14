#ifndef _ERRMARK_H
#define _ERRMARK_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <sys/types.h>

extern void mark_open(void);
extern void mark_close(void);
extern bool parse_mark_specs(char *);
extern bool setmark(int fd, char const *m_start, char const *m_end);
extern void before_write(int fd, void *buf, size_t len);
extern void after_write(int fd, void *buf, size_t len);

#include <stdio.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#if defined(__ILP64__)
#include <sys/reg.h>
#endif

#include <sys/syscall.h>

struct cmd {
    int argc;
    char * const *argv;
    const char *cmd_path;
    const char *cmd_name;

    // Options
    bool verbose;
    bool debug;
    bool slow;
    FILE *trace_fbt;
    bool nullify;

    char *copy_fname;
    FILE *copy_fh;

    // State
    int  mark_state;
    pid_t child;
    int child_status;
    int rc;
    bool child_exited;
};

typedef struct cmd cmd_t;

extern long guard_ptrace(cmd_t *, enum __ptrace_request request, pid_t pid, void *addr, void *data);

extern ssize_t pmem_fwrite(FILE *f, pid_t tracee, void *raddr, size_t len);
extern ssize_t pmem_copy(char *buf, pid_t tracee, void *raddr, size_t len);

extern int errmark_run_program(cmd_t *);

#ifdef  __cplusplus
}
#endif

#endif /* _ERRMARK_H */
