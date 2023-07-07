/* search.c - search good moves.
 *
 * Copyright (C) 2023 Bruno Raoult ("br")
 * Licensed under the GNU General Public License v3.0 or later.
 * Some rights reserved. See COPYING.
 *
 * You should have received a copy of the GNU General Public License along with this
 * program. If not, see <https://www.gnu.org/licenses/gpl-3.0-standalone.html>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later <https://spdx.org/licenses/GPL-3.0-or-later.html>
 *
 */


#include <br.h>
#include <list.h>
#include <debug.h>

#include "move.h"
#include "eval.h"
#include "search.h"

/**
 * negamax() - the negamax tree.
 *
 */
eval_t negamax(pos_t *pos, int depth)
{
    move_t *move, *bestmove;
    pos_t *newpos;
    eval_t best = EVAL_MIN, score;

    moves_gen_all(pos);
    if (depth == 0)
        return eval(pos) * pos->turn == WHITE? 1: -1;

    list_for_each_entry(move, &pos->moves[pos->turn], list) {
        newpos = move_do(pos, move);
        score = -negamax(newpos, depth - 1 );
        if(score > best) {
            best = score;
            bestmove = move;
#           ifdef DEBUG_SEARCH
            log_f(2, "depth=%d best move=", best);
            move_print(0, bestmove, M_PR_LONG);
            log_f(2, " eval=%d\n", best);
#           endif
        }
    }
    return best;
}
