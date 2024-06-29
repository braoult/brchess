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
    { { .move = MOVE_NONE, .key = U64(0), .prev = &hist.state[0] } },
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
    hist.state[0].move = MOVE_NONE;
    hist.state[0].prev = &hist.state[0];
}

/**
 * hist_push() - add a state to hist list
 * @st:   &state_t to add
 *
 * Used to add moves when the UCI "position" command includes moves.
 * These moves, but last one, should be pushed. Last move should be
 * linked to hist with @hist_link().
 */
void hist_push(state_t *st) //, move_t *move)
{
    int last = hist.nstates++;

    bug_on(last >= HIST_SIZE);
    hist.state[last] = *st;
    hist.state[last].prev = &hist.state[last - 1];
    // hist.state[last].move = *move;
}

/**
 * hist_link() - link a position to last hist element.
 * @pos:   &pos_t to link
 *
 * Used to add position resulting from last "move" in UCI "position" command.
 * All other moves in UCI "position" command should be pushed instead, with
 * hist_push().
 */
void hist_link(pos_t *pos)
{
    pos->prev = hist_last();
}

/**
 * hist_pop() - return last state from hist entry, and remove it from list
 *
 * Not used, only for debug.
 */
state_t *hist_pop(void)
{
    if (hist.nstates > 1)
        hist.nstates--;
    return hist_last();
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
 * hist_static_print() - print hist entries
 */
void hist_static_print(void)
{
    char movestr[8];
    state_t *st =  hist_last();

    printf("UCI state history: ");
    while (true) {
        printf("%s(#%lx) ",
               move_to_str(movestr, st->move, 0),
               hash_short(st->key));
        if (st == HIST_START)
            break;
        st = hist_prev(st);
    }
    printf("\n");
}

/**
 * hist_print() - print position history
 * @pos: &pos to start from
 */
void hist_print(pos_t *pos)
{
    char movestr[8];
    state_t *st = &pos->state;
    printf("position states history: ");
    while (true) {
        printf("%s(#%lx) ",
               move_to_str(movestr, st->move, 0),
               hash_short(st->key));
        if (st == HIST_START)
            break;
        st = hist_prev(st);
    }
    printf("\n");
}
