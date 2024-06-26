/* misc.c - generic/catchall functions.
 *
 * Copyright (C) 2024 Bruno Raoult ("br")
 * Licensed under the GNU General Public License v3.0 or later.
 * Some rights reserved. See COPYING.
 *
 * You should have received a copy of the GNU General Public License along with this
 * program. If not, see <https://www.gnu.org/licenses/gpl-3.0-standalone.html>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later <https://spdx.org/licenses/GPL-3.0-or-later.html>
 *
 */

#include <time.h>
#include <stdlib.h>

#include <bug.h>

#include "chessdefs.h"

/*
 * 1 sec      = 1000 millisec
 * 1 millisec = 1000 microsec
 * 1 microsec = 1000 nanosec
 * milli = sec * 1000 + nanosec / 1000000
 *
 */

/* We use microsec for all intermediate calcluation */
#define NANO_IN_MICRO  1000ll                     /* nanosecond in millisecond */
#define MICRO_IN_SEC   1000000ll                  /* millisecond in second */

#define MILLI_IN_SEC   1000ll                     /* millisecond in second */

#define MICRO_IN_MILLI 1000ll

//#define NANO_IN_MILLI  1000000ll                         /* nanosecond in millisecond */
//#define NANO_IN_SEC    (NANOS_IN_MS * MS_IN_SEC)


/**
 * clock_start - start or restart a clock.
 * @clock: &mclock_t clock
 *
 * Save current time according to @clock type.
 */
void clock_start(mclock_t *clock)
{
    clock_gettime(clock->clocktype, &clock->start);
}

/**
 * clock_init - initializes a clock type.
 * @clock: &mclock_t clock
 * @type: clock type
 *
 * See the clock_gettime(2) for details.
 * CLOCK_WALL (a.k.a CLOCK_REALTIME): Wall clock.
 * CLOCK_SYSTEM (a.k.a CLOCK_MONOTONIC_RAW): System clock.
 * CLOCK_PROCESS (a.k.a CLOCK_PROCESS_CPUTIME_ID): Process CPU clock (incl. threads).
 * CLOCK_THREAD  (a.k.a CLOCK_THREAD_CPUTIME_ID): Thread CPU clock.
 */
void clock_init(mclock_t *clock, clockid_t type)
{
    clock->clocktype = type;
    clock_start(clock);
}

/**
 * clock_elapsed_μs - return a mclock_t elapsed time in microseconds.
 * @clock: &mclock_t clock
 *
 * The elapsed time is calculated between current time and last clock_start(@clock)
 * call time.
 *
 * @return: microseconds elapsed since last clock_start().
 */
s64 clock_elapsed_μs(mclock_t *clock)
{
    struct timespec current;
    s64 μs;

    clock_gettime(clock->clocktype, &current);
    μs = ((s64)current.tv_sec - (s64)clock->start.tv_sec) * MICRO_IN_SEC +
        ((s64)current.tv_nsec - (s64)clock->start.tv_nsec) / NANO_IN_MICRO ;
    return μs;
}

/**
 * clock_elapsed_ms - return a mclock_t elapsed time in milliseconds.
 * @clock: &mclock_t clock
 *
 * The elapsed time is calculated between current time and last clock_start(@clock)
 * call time.
 *
 * @return: milliseconds elapsed since last clock_start().
 */
s64 clock_elapsed_ms(mclock_t *clock)
{
    return clock_elapsed_μs(clock) / MICRO_IN_MILLI;
}

/**
 * clock_elapsed_sec - return a mclock_t elapsed time in seconds.
 * @clock: &mclock_t clock
 *
 * The elapsed time is calculated between current time and last clock_start(@clock)
 * call time.
 *
 * @return: seconds elapsed since last clock_start().
 */
double clock_elapsed_sec(mclock_t *clock)
{
    return (double) clock_elapsed_μs(clock) / (double) MICRO_IN_SEC;
}

static u64 rand_seed = 1ull;

/**
 * rand_init() - initialize random generator seed.
 * @seed: u64, the random generator seed.
 *
 * No change is made is performed if If @seed is zero. By default, @seed is
 * 1.
 * This seed is used by rand64().
 */
void rand_init(u64 seed)
{
    if (seed)
        rand_seed = seed;
}

/**
 * rand64() - get a random number, xorshift method.
 *
 * Source:
 *   https://en.wikipedia.org/wiki/Xorshift#xorshift*
 * We do not want true random numbers, like those offered by getrandom(2), as we
 * need to be able to get predictable results.
 * Note: For predictable results in MT, we should use separate seeds.
 *
 * @return: a 64 bits random number.
 */
u64 rand64(void)
{
    bug_on(rand_seed == 0ull);

    rand_seed ^= rand_seed >> 12;
    rand_seed ^= rand_seed << 25;
    rand_seed ^= rand_seed >> 27;
    return rand_seed * 0x2545f4914f6cdd1dull;
}
