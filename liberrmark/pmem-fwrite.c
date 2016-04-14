#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>		// Import size_t
#include <stdint.h>		// Import uintptr_t
#include <errno.h>		// Import errno, EIO
#include <sys/ptrace.h>		// Import PTRACE_PEEKDATA

/**
 * @brief write a region of data from the process being traced.
 *
 * Get data from the process being traced, at the given address,
 * and write it to a file.
 *
 * This could be done in two steps: read the entire memory region
 * into a buffer in our own address space; then write our copy
 * of the data.  But, if we are not going to do anything else with
 * the data, then we may as well write it one word at a time,
 * directly from the tracee's address space.
 *
 * @param f       output stream
 * @param tracee  pid of process being traced
 * @param raddr   address of region of data
 * @param len     size in bytes of the region
 * @return        number of bytes actually read successfully
 */

ssize_t
pmem_fwrite(FILE *f, pid_t tracee, void *raddr, size_t len)
{
    union {
        long iword;
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
        fwrite(data.bytes, sz, 1, f);
        raddr = (void *)((char *)raddr + sz);
        len -= sz;
    }

    /*
     * Write whole words (long).
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
        fwrite(data.bytes, sizeof (data), 1, f);
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
        fwrite(data.bytes, len, 1, f);
        bytes_read += len;
    }

    fflush(f);
    return (bytes_read);
}
