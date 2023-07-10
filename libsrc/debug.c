/* debug.c - debug/log management
 *
 * Copyright (C) 2021-2022 Bruno Raoult ("br")
 * Licensed under the GNU General Public License v3.0 or later.
 * Some rights reserved. See COPYING.
 *
 * You should have received a copy of the GNU General Public License along with this
 * program. If not, see <https://www.gnu.org/licenses/gpl-3.0-standalone.html>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later <https://spdx.org/licenses/GPL-3.0-or-later.html>
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#ifndef DEBUG_DEBUG
#define DEBUG_DEBUG
#endif

#include "bits.h"
#include "debug.h"

#define NANOSEC  1000000000                       /* nano sec in sec */
#define MILLISEC 1000000                          /* milli sec in sec */

static long long timer_start;                     /* in nanosecond */
static uint debug_level = 0;
static bool debug_flush = false;
static FILE *stream = NULL;

/**
 * debug_level_set() - set debug level.
 * @level: unsigned debug level.
 */
void debug_level_set(uint level)
{
    debug_level = level;
#   ifdef DEBUG_DEBUG_C
    log(0, "debug level set to %u\n", level);
#   endif
}

/**
 * debug_level_get() - get debug level.
 * @return: current level debug (unsigned integer).
 */
uint debug_level_get(void)
{
    return debug_level;
}

void debug_stream_set(FILE *_stream)
{
    stream = _stream;
#   ifdef DEBUG_DEBUG_C
    log(0, "stream set to %d\n", stream? fileno(stream): -1);
#   endif
}

void debug_flush_set(bool flush)
{
    debug_flush = flush;
#   ifdef DEBUG_DEBUG_C
    log(0, "debug flush %s.\n", flush? "set": "unset");
#   endif
}

void debug_init(uint level, FILE *_stream, bool flush)
{
    struct timespec timer;

    debug_stream_set(_stream);
    debug_level_set(level);
    debug_flush_set(flush);
    if (!clock_gettime(CLOCK_MONOTONIC, &timer)) {
        timer_start = timer.tv_sec * NANOSEC + timer.tv_nsec;
    }
    else {
        timer_start = 0;
    }
    log(0, "timer started.\n");
}

inline static long long timer_elapsed()
{
    struct timespec timer;

    clock_gettime(CLOCK_MONOTONIC, &timer);
    return (timer.tv_sec * NANOSEC + timer.tv_nsec) - timer_start;
}

/**
 * debug() - log function
 * @level: log level
 * @timestamp: boolean, print timestamp if true
 * @indent: indent level (2 spaces each)
 * @src: source file/func name (or NULL)
 * @line: line number
 */
void debug(uint level, bool timestamp, uint indent, const char *src,
           uint line, const char *fmt, ...)
{
    if (!stream || level > debug_level)
        return;

    va_list ap;

    if (indent)
        fprintf(stream, "%*s", 2*(indent-1), "");

    if (timestamp) {
        long long diff = timer_elapsed();
        fprintf(stream, "%lld.%03lld ", diff/NANOSEC, (diff/1000000)%1000);
        fprintf(stream, "%010lld ", diff);
    }

    if (src) {
        if (line)
            fprintf(stream, "[%s:%u] ", src, line);
        else
            fprintf(stream, "[%s] ", src);
    }
    va_start(ap, fmt);
    vfprintf(stream, fmt, ap);
    va_end(ap);
    if (debug_flush)
        fflush(stream);
}

#ifdef BIN_debug
#include <unistd.h>

int main()
{
    int foo=1;
    debug_init(5);

    log(0, "log0=%d\n", foo++);
    log(1, "log1=%d\n", foo++);
    log(2, "log2=%d\n", foo++);
    log_i(2, "log_i 2=%d\n", foo++);
    log_i(5, "log_i 5=%d\n", foo++);
    log_i(6, "log_i 6=%d\n", foo++);
    log_it(4, "log_it 4=%d\n", foo++);
    log_f(1, "log_f 5=%d\n", foo++);
}
#endif
