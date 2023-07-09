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
eval_t negamax(pos_t *pos, int depth, int color)
{
    move_t *move;
    pos_t *newpos;
    eval_t best = EVAL_MIN, score;

    printf("depth=%d\n", depth);
    moves_gen_all(pos);
    //pos_check(pos);
    if (depth == 0) {
        score = eval(pos);
        //printf("evalnega=%d turn=%d color=%d", score, pos->turn, color);
        score *= color;
        //printf(" --> evalnega=%d\n", score);
        return score;
    }
    moves_print(pos, 0);
    list_for_each_entry(move, &pos->moves[pos->turn], list) {
        newpos = move_do(pos, move);
        score = -negamax(newpos, depth - 1, -color);
        move->negamax = score;
        //printf("move=");
        //move_print(0, move, 0);
        //printf("score=%d\n", score);

        if (score > best) {
            best = score;
            pos->bestmove = move;
#           ifdef DEBUG_SEARCH
            //printf("depth=%d best move=", depth);
            //move_print(0, &bestmove, 0);
            //printf(" eval=%d\n", best);
#           endif
        }
        move_undo(newpos, move);
    }
    return best;
}
