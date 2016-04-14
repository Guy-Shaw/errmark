#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>		// Import size_t
#include <stdint.h>		// Import uintptr_t
#include <errno.h>		// Import errno, EIO
#include <sys/ptrace.h>		// Import PTRACE_PEEKDATA

#include <string.h>             // Import memcpy()

/**
 * @brief Copy a region of data from the process being traced to allocated memory
 *
 * Get data from the process being traced, at the given address,
 * and copy it to a buffer.
 *
 * @param buf     destination buffer, already allocated to hold len bytes.
 * @param tracee  pid of process being traced
 * @param raddr   address of region of data
 * @param len     size in bytes of the region
 * @return        number of bytes actually read successfully
 */

ssize_t
pmem_copy(char *buf, pid_t tracee, void *raddr, size_t len)
{
    union {
        long   iword;
        char bytes[sizeof (size_t)];
    } data;
    size_t phase;
    size_t bytes_read;

    bytes_read = 0;

    /*
     * Take care of any possible unaligned data at the start.
     */
    phase = ((uintptr_t)raddr & (sizeof (data) - 1));
    if (phase != 0) {
        size_t sz;
        size_t remsz;

        remsz = sizeof (data) - phase;
        sz = (len > remsz) ? remsz : len;
        errno = 0;
        data.iword = ptrace(PTRACE_PEEKDATA, tracee, raddr, NULL);
        if (data.iword == -1 && errno != 0) {
            if (bytes_read != 0 && errno == EIO) {
                return (bytes_read);
            }
            else {
                return (-1);
            }
        }
        bytes_read = sz;
        memcpy(buf, data.bytes, sz);
        buf += sz;
        raddr = (void *)((char *)raddr + sz);
        len -= sz;
    }

    /*
     * Write whole words (sizeof (size_t)).
     */
    while (len >= sizeof (data)) {
        errno = 0;
        data.iword = ptrace(PTRACE_PEEKDATA, tracee, raddr, NULL);
        if (data.iword == -1 && errno != 0) {
            if (bytes_read != 0 && errno == EIO) {
                return (bytes_read);
            }
            else {
                return (-1);
            }
        }
        memcpy(buf, data.bytes, sizeof (data));
        buf += sizeof (data);
        raddr = (void *)((char *)raddr + sizeof (data));
        len -= sizeof (data);
        bytes_read += sizeof (data);
    }

    /*
     * Take care of any runt word at the end.
     */
    if (len > 0) {
        errno = 0;
        data.iword = ptrace(PTRACE_PEEKDATA, tracee, raddr, NULL);
        if (data.iword == -1 && errno != 0) {
            if (bytes_read != 0 && errno == EIO) {
                return (bytes_read);
            }
            else {
                return (-1);
            }
        }
        memcpy(buf, data.bytes, len);
        bytes_read += len;
    }

    return (bytes_read);
}
