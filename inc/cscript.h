/*
 * Filename: cscript.h
 * Library: libcscript
 * Brief: Generally handly headers for most libcscript code
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

#ifndef _CSCRIPT_H
#define _CSCRIPT_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdio.h>
#include <sys/types.h>      // Import size_t

typedef unsigned int uint_t;

// eprint.h

extern FILE *errprint_fh;

#define eprint(str) fputs((str), errprint_fh)
#define eputchar(c) putc((c), errprint_fh)

#if defined(__GNUC__)

#define eprintf(fmt, ...) \
    fprintf(errprint_fh, (fmt), ## __VA_ARGS__)

#else

extern int eprintf(const char *, ...);

#endif /* __GNUC__ */

// dbgprint.h

extern bool debug;

extern FILE *dbgprint_fh;

#define dbg_print(str) fputs((str), dbgprint_fh)
#define dbg_putchar(c) putc((c), dbgprint_fh)

#if defined(__GNUC__)

#define dbg_printf(fmt, ...) \
    ({ if (debug) { fprintf(dbgprint_fh, (fmt), ## __VA_ARGS__); }; })

#else

extern int dbgprintf(const char *, ...);

#endif /* __GNUC__ */

#define HERE \
    dbg_printf("\n===> %s, line %u\n", __FILE__, __LINE__)
#define HERE_FN \
    dbg_printf("\n===> %s, line %u, %s\n", __FILE__, __LINE__, __FUNCTION__)

// ==================== Iterate over a list of files

#ifdef IMPORT_FVH


// filev_handle iterator

struct fvh {
    uint_t filec;
    char **filev;
    FILE *fh;   // Initially NULL
    uint_t fnr;
    char *fname;
    char *line;
    char *endl;
    uint_t glnr;
    uint_t flnr;
    int eof;
};

typedef struct fvh fvh_t;

#endif /* IMPORT_FVH */

// ==================== libcscript external functions

extern int    set_print_fh(void);
extern int    filev_probe(uint_t filec, char **filev);
extern int    filev_by_char(uint_t filec, char **filev);
extern int    filev_by_rune(uint_t filec, char **filev);
extern int    filev_by_word(uint_t filec, char **filev);
extern int    filev_by_line(uint_t filec, char **filev);
extern int    filev_by_paragraph(uint_t filec, char **filev);
extern void   fshow_str_array(FILE *, uint_t, char * const *);
extern size_t fshow_str(FILE *, char *);
extern void   fshow_errno(FILE *f, const char *msg, int err);
extern void   fshow_fname(FILE *f, const char *fname);
extern void   fshow_wait_status(FILE *, const char *, int);
extern const char * sname(const char *);
extern char * decode_esym_r(char *buf, size_t sz, int err);
extern ssize_t qp_decode_str(char *buf, size_t sz, const char *str);
extern ssize_t qp_encode_str(char *buf, size_t sz, char *str);
extern int     close_from(int fd);

extern void   fshow_svar(FILE *f, const char *var, const char *value);
extern void   dbg_show_svar(const char *var, const char *value);
extern void * guard_malloc(size_t sz);
extern void * guard_calloc(size_t nelem, size_t sz);
extern void   fexpain_err(FILE *f, int err);
extern void   eexpain_err(int err);
extern void   expain_err(int err);
extern int    file_test(const char *tests, const char *fname);

// Note: msg is _not_ of type @type{const char *}, because the message
// is modified.  Some spaces or punctuation are changed to '\0',
// in order to terminate strings.
//
// This may change, when we convert to "Better String Library" (bstring),
// or something like it.
//
extern void explain_fmt_fopen(char *msg);

extern void fhrule(FILE *f);
extern void ferror_msg_start(FILE *f);
extern void ferror_msg_finish(FILE *f);
extern void error_msg_start(void);
extern void error_msg_finish(void);

extern ssize_t size_to_ssize(size_t sz);

struct color_esc {
    char const *name;
    char const *esc_start;
    char const *esc_end;
};

typedef struct color_esc color_esc_t;

extern color_esc_t *lookup_color(char const *color_name);

#ifdef  __cplusplus
}
#endif

#endif  /* _CSCRIPT_H */