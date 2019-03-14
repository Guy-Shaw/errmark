#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE 1

#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include <cscript.h>

extern FILE *dbgprint_fh;

extern bool verbose;
extern bool debug;

static char *start1 = NULL;
static char *end1   = NULL;
static char *start2 = NULL;
static char *end2   = NULL;

static int cur_fd = -1;

/*
 * Parse a --mark option, and set start/end triggers
 * for the given file descriptors.
 *
 * A mark specification is a file descriptor (1 or 2),
 * followed by a separator character, followed by a start string,
 * then another separator character (the same as the first),
 * then an end string.
 *
 * Example: --mark '1:[[:]]:'
 */
bool
parse_mark_specs(char *mspec)
{
    char *s;
    char *mark_start;
    char *mark_end;
    size_t mark_start_len;
    size_t mark_end_len;
    int fsep;
    int fd;

    s = mspec;
    mark_start = NULL;
    mark_end   = NULL;
    mark_start_len = 0;
    mark_end_len   = 0;
    if (*s == '1' || *s == '2') {
        fd = *s - '0';
        ++s;
        fsep = *s;
    }
    else {
        return (false);
    }

    if (*s == '\0') {
        return (true);
    }

    ++s;
    mark_start = s;
    while (*s && *s != fsep) {
        ++s;
    }
    mark_start_len = s - mark_start;

    if (*s == fsep) {
        ++s;
        mark_end = s;
        while (*s) {
            ++s;
        }
        if (*s == fsep) {
            --s;
        }
        mark_end_len = s - mark_end;
    }

    if (mark_start_len == 0) {
        mark_start = NULL;
    }

    if (mark_end_len == 0) {
        mark_end = NULL;
    }

    if (fd == 1) {
        start1 = strndup(mark_start, mark_start_len);
        end1   = strndup(mark_end, mark_end_len);
        if (debug) {
            fprintf(dbgprint_fh, "start1=[");
            fshow_str(dbgprint_fh, start1);
            fprintf(dbgprint_fh, "]\n");
            fprintf(dbgprint_fh, "end1  =[");
            fshow_str(dbgprint_fh, end1);
            fprintf(dbgprint_fh, "]\n");
        }
    }
    if (fd == 2) {
        start2 = strndup(mark_start, mark_start_len);
        end2   = strndup(mark_end, mark_end_len);
        if (debug) {
            fprintf(dbgprint_fh, "start2=[");
            fshow_str(dbgprint_fh, start2);
            fprintf(dbgprint_fh, "]\n");
            fprintf(dbgprint_fh, "end2  =[");
            fshow_str(dbgprint_fh, end2);
            fprintf(dbgprint_fh, "]\n");
        }
    }

    return (true);
}

bool
setmark(int fd, char const *m_start, char const *m_end)
{
    if (fd == 1) {
        start1 = strdup(m_start);
        end1   = strdup(m_end);
    }
    else if (fd == 2) {
        start2 = strdup(m_start);
        end2   = strdup(m_end);
    }
    else {
        fprintf(stderr, "fd=%d -- only fd 1 or 2 are supported.\n", fd);
        return (false);
    }

    return (true);
}

void
write_start_stdout(void)
{
    if (start1 != NULL) {
        fflush(stdout);
        fflush(stderr);
        write(1, start1, strlen(start1));
    }
}

void
write_end_stdout(void)
{
    if (end1 != NULL) {
        fflush(stdout);
        fflush(stderr);
        write(1, end1, strlen(end1));
    }
}

void
write_start_stderr(void)
{
    if (start2 != NULL) {
        fflush(stdout);
        fflush(stderr);
        write(2, start2, strlen(start2));
    }
}

void
write_end_stderr(void)
{
    if (end2 != NULL) {
        fflush(stdout);
        fflush(stderr);
        write(2, end2, strlen(end2));
    }
}

void
switch_from_fd(int fd)
{
    if (fd == 1) {
        write_end_stdout();
    }
    if (fd == 2) {
        write_end_stderr();
    }
}

void
switch_to_fd(int fd)
{
    if (fd == 1) {
        write_start_stdout();
    }
    if (fd == 2) {
        write_start_stderr();
    }
}

void
before_write(int fd, void *buf, size_t len)
{
    if (buf == NULL || len == (size_t)(-1)) {
        return;
    }

    if ((fd == 1 || fd == 2) && fd != cur_fd) {
        switch_from_fd(cur_fd);
        switch_to_fd(fd);
        cur_fd = fd;
    }
}

void
after_write(int fd, void *buf, size_t len)
{
    if (!(fd == 1 || fd == 2)) {
        return;
    }

    if (buf == NULL || len == (size_t)(-1)) {
        return;
    }

    /*
     * Do nothing.
     * Only write ending strings on transition to/from stdout/stderr.
     */
}

void
mark_open(void)
{
    cur_fd = -1;
}

void
mark_close(void)
{
    switch_from_fd(cur_fd);
}
