/* util.h - various util functions.
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

#ifndef _UTIL_H
#define _UTIL_H

#include <brlib.h>

#include <time.h>

typedef struct mclock {
    clockid_t clocktype;
    ulong elapsed_l;
    double elapsed_f;
    struct timespec start;
} mclock_t;

#define CLOCK_WALL    CLOCK_REALTIME
#define CLOCK_SYSTEM  CLOCK_MONOTONIC_RAW
#define CLOCK_PROCESS CLOCK_PROCESS_CPUTIME_ID
#define CLOCK_THREAD  CLOCK_THREAD_CPUTIME_ID

/**
 * CLOCK_DEFINE - define a clock type.
 * @name: clock name
 * @type: clock type
 *
 * This macro is equivalent to:
 *   mclock_t name;
 *   clock_init(&name, type);
 */
#define CLOCK_DEFINE(name, type) struct mclock name = { .clocktype = type }

void clock_init(mclock_t *clock, clockid_t type);
void clock_start(mclock_t *clock);
s64 clock_elapsed_μs(mclock_t *clock);
s64 clock_elapsed_ms(mclock_t *clock);
double clock_elapsed_sec(mclock_t *clock);

#define RAND_SEED_DEFAULT U64(0xb0d1ccea)

void rand_init(u64 seed);
u64 rand64(void);


bool str_eq_case(char *str1, char *str2);
char *str_trim(char *str);
char *str_token(char *str, const char *token);
char *str_skip_word(char *str);

#endif  /* UTIL_H */
