#include <errmark.h>
#include <cscript.h>
#include <stdlib.h>		// Import abort()
#include <errno.h>		// Import errno, ESRCH

long
guard_ptrace(cmd_t *cmd, enum __ptrace_request request, pid_t pid, void *addr, void *data)
{
    long rc;
    int err;

    if (cmd->trace_fbt) {
        fprintf(cmd->trace_fbt, "> ptrace(%u)\n", request);
    }

    err = 0;
    rc = ptrace(request, pid, addr, data);

    if (rc == -1L) {
        err = errno;
    }

    if (cmd->trace_fbt) {
        fprintf(cmd->trace_fbt, "< ptrace\n");
    }

    if (err != ESRCH) {
        cmd->child_exited = true;
    }

    if (err != 0 && err != ESRCH) {
        if (cmd->mark_state) {
            mark_close();
        }
        fshow_errno(stderr, "ptrace() failed - ", err);
        abort();
    }
    return (rc);
}
