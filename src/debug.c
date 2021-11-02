/* debug.c - debug/log management
 *
 * Copyright (C) 2021 Bruno Raoult ("br")
 * Licensed under the GNU General Public License v3.0 or later.
 * Some rights reserved. See COPYING.
 *
 * You should have received a copy of the GNU General Public License along with this
 * program. If not, see <https://www.gnu.org/licenses/gpl-3.0-standalone.htmlL>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later <https://spdx.org/licenses/GPL-3.0-or-later.html>
 *
 */

//#ifdef DEBUG
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "debug.h"

#define NANOSEC  1000000000                       /* nano sec in sec */
#define MILLISEC 1000000                          /* milli sec in sec */

static int64_t timer_start;                       /* in nanosecond */

int clock_gettime(clockid_t clockid, struct timespec *tp);

void debug_init()
{
    struct timespec timer;
    if (!clock_gettime(CLOCK_MONOTONIC, &timer)) {
        timer_start = timer.tv_sec * NANOSEC + timer.tv_nsec;
    }
    else {
        timer_start = 0;
    }
    log("timer started.\n");
}

inline static int64_t timer_elapsed()
{
    struct timespec timer;

    clock_gettime(CLOCK_MONOTONIC, &timer);
    return (timer.tv_sec * NANOSEC + timer.tv_nsec) - timer_start;
}


/* void debug - log function
 * @timestamp : boolean
 * @indent    : indent level (2 spaces each)
 * @src       : source filename (or NULL)
 * @line      : line number
 */
void debug(bool timestamp, uint32_t indent, const char *src,
           uint32_t line, const char *fmt, ...)
{
    va_list ap;

    if (indent)
        printf("%*s", 2*indent, "");

    if (timestamp) {
        int64_t diff = timer_elapsed();
        printf("%ld.%03ld ", diff/NANOSEC, (diff/1000000)%1000);
        printf("%010ld ", diff);
    }

    if (src) {
        if (line)
            printf("[%s:%u] ", src, line);
        else {
            printf("[%s] ", src);
        }
    }
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}

#ifdef DEBUGBIN
#include <unistd.h>

int main()
{
    int foo=1;
    debug_init();

    log("log=%d\n", foo++);
    log_i(1, "log_i=%d\n", foo++);
    log_i(2, "log_i=%d\n", foo++);
    log_it(3, "log_it=%d\n", foo++);
    log_f("log_f=%d\n", foo++);
}
#endif

//#endif /* DEBUG_H */
