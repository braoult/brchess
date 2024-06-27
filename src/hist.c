/* hist.c - history management.
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

#include <brlib.h>
#include <bug.h>

#include "position.h"
#include "hash.h"
#include "move.h"
#include "hist.h"

hist_t hist = {
    .nstates = 1,
    { { .move = MOVE_NO_MOVE, .key = U64(0), .prev = &hist.state[0] } },
};

/**
 * hist_init() - initialize history states data.
 *
 * This should be done every time a new position must be handled.
 */
void hist_init(void)
{
    hist.nstates       = 1;
    hist.state[0].key  = U64(0);
    hist.state[0].move = MOVE_NO_MOVE;
    hist.state[0].prev = &hist.state[0];
}

/**
 * hist_push() - add a state and move to hist
 * @st:   &state_t to add
 * @move: &move_t to add in @st
 */
void hist_push(state_t *st, move_t *move)
{
    int last = hist.nstates++;

    bug_on(last >= HIST_SIZE);
    hist.state[last] = *st;
    hist.state[last].prev = &hist.state[last - 1];
    hist.state[last].move = *move;
}

/**
 * hist_pop() - return last state from hist entry, and remove it from list
 *
 * Not used, only for debug.
 */
state_t *hist_pop(void)
{
    return hist.state + --hist.nstates;
}

/**
 * hist_last() - return last state from hist.
 */
state_t *hist_last(void)
{
    return hist.state + hist.nstates - 1;
}

/**
 * hist_prev() - return a state's ancestor.
 * @st: &state_t state
 *
 * No test is done on ancestor. Caller should check it is different
 * from HIST_START.
 */
state_t *hist_prev(state_t *st)
{
    return st->prev;
}

/**
 * hist_prev2() - return a state's second ancestor.
 * @st: &state_t state
 *
 * No test is done on ancestors. Caller should check it is different
 * from HIST_START.
 */
state_t *hist_prev2(state_t *st)
{
    return st->prev->prev;
}

/**
 * hist_prev4() - return a state's 4th ancestor.
 * @st: &state_t state
 *
 * No test is done on ancestors. Caller should check it is different
 * from HIST_START.
 */
state_t *hist_prev4(state_t *st)
{
    return st->prev->prev->prev->prev;
}

/**
 * hist_print() - print hist entries
 */
void hist_static_print(void)
{
    char movestr[8];

    printf("rev history: ");
    for (state_t *p = hist_last(); p != HIST_START; p = hist_prev(p)) {
        printf("%s(%lx) ",
               p->move == MOVE_NO_MOVE? "none": move_to_str(movestr, p->move, 0),
               hash_short(p->key));
    }
    printf("\n");
}
