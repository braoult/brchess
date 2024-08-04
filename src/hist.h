/* hist.h - history management.
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

#ifndef HIST_H
#define HIST_H

#include <brlib.h>
#include <bug.h>

#include "position.h"
#include "hash.h"

#define HIST_SIZE 4096                            /* I know, I know... */

/**
 * hist - history states data.
 *
 * This variable is of type hist_t, which contains:
 * @state:    state_t array, size HIST_SIZE
 * @nstates:  current number of @state
 *
 * hist contains moves already played.
 *
 * Only HIST_SIZE - 1 hist elements are available, as the first element is
 * used as a sentinel value (hist[0].state.prev = &hist[0].state).
 * This first element allows multiple backards searches (p->prev->prev).
 *
 * hist is only written by main thread, and read by other threads/processes,
 * therefore is never duplicated (even after a fork(), due to COW).
 */
typedef struct {
    int nstates;
    state_t state[HIST_SIZE];
} hist_t;

extern hist_t hist;

#define HIST_START  (hist.state)

void hist_init(void);
state_t *hist_next(void);
void hist_push(state_t *st); //, move_t *move);
void hist_link(pos_t *pos);
state_t *hist_pop(void);
state_t *hist_last(void);
state_t *hist_prev(state_t *st);
state_t *hist_prev2(state_t *st);
state_t *hist_prev4(state_t *st);
void hist_static_print(void);
void hist_print(pos_t *pos);

#endif  /* HIST_H */
