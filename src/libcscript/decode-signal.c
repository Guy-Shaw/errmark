/*
 * Filename: decode-signal.c
 * Library: libcscript
 * Brief: Given a signal number, return the symbolic name of that signal
 *
 * Description:
 *   Given a signal number, return a string, which is the symbolic
 *   name of that signal as defined in _sys_siglist[sig].  If the
 *   signal number is not valid, then the resulting string is 
 *   "Unknown signal " followed by the signal number.
 *
 * Copyright (C) 2016 Guy Shaw
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

#if defined(__GNUC__) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE 1
#endif

#include <stdio.h>		// Import snprintf()
#include <signal.h>
#include <sys/types.h>		// Import size_t
#include <assert.h>

/**
 *
 * @brief Return the symbolic name of the given signal number.
 *
 * @param buf  IN,OUT  String buffer that _may_ be used to hold the result.
 * @param sz   IN      The capacity of |buf|.
 * @param sig  IN      The signal number to be decoded.
 * @return A string symbolically explaining |sig|.
 *
 * decode_signal_r() will _always_ return a pointer to a string,
 * so it can be used directly in expressions where a char * is needed.
 * For example, in printf-like functions (but only once).
 *
 * There is _never_ an error.  If the given signal value is invalid,
 * a descriptive string is still returned.  It just indicates the
 * error and the numeric value.
 *
 * Normally, the decoded signal is _not_ copied into |buf|.
 * In the case of known errno values, decode_signal_r() returns a pointer
 * to a static string, which is a simple identifier, that is defined
 * to be the numeric value, |sig|, according to the definitions
 * in the global array _sys_siglist[].
 *
 * The destination buffer is used only in the case of unknown signal values.
 * This is when a string value must be computed.
 * So, the returned pointer is always what should be used, not |buf|.
 *
 */
char *
decode_signal_r(char *buf, size_t sz, int sig)
{
    const char *desc;
    const char *ufmt = "Unknown signal %d";
    size_t len;

    if (sig >= 0 && sig < NSIG && ((desc = _sys_siglist[sig]) != NULL)) {
        return ((char *)desc);
    }

#ifdef SIGRTMIN
    if (sig >= SIGRTMIN && sig <= SIGRTMAX) {
        sig = sig - SIGRTMIN;
        ufmt = "Real-time signal %d";
    }
#endif

    len = snprintf(buf, sz - 1, ufmt, sig);
    assert(len < sz - 1);
    return (buf);
}
