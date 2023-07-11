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
 * negamax() - search position negamax.
 * @pos: &position to search
 * @depth: Wanted depth.
 * @color: 1 for white, -1 for black.
 *
 * Calculate the negamax value of @pos. This is an extensive search, with
 * absolutely no cutoff.
 *
 * @return: The @pos negamax evaluation.
 */
eval_t negamax(pos_t *pos, int depth, int color)
{
    move_t *move;
    pos_t *newpos;
    eval_t best = EVAL_MIN, score;

    if (depth == 0) {
        moves_gen_all_nomoves(pos);
        score = eval(pos) * color;
        return score;
    }
    moves_gen_all(pos);
    list_for_each_entry(move, &pos->moves[pos->turn], list) {
        log(1, "%.*s", 5 - depth, "                ");
        newpos = move_do(pos, move);
        score = -negamax(newpos, depth - 1, -color);
        pos->node_count += newpos->node_count;
        move->negamax = score;
        if (score > best) {
            best = score;
            pos->bestmove = move;
        }
        move_undo(newpos, move);
    }
    return best;
}


/**
 * pvs() - Principal Variation Search.
 * @pos: &position to search
 * @depth: wanted depth.
 * @alpha: alpha value.
 * @beta: beta value.
 * @color: 1 for white, -1 for black.
 *
 * Calculate the PVS value of @pos.
 * See https://en.wikipedia.org/wiki/Principal_variation_search
 *
 * @return: The @pos PVS evaluation.
 */
eval_t pvs(pos_t *pos, int depth, int alpha, int beta, int color)
{
    move_t *move;
    pos_t *newpos;
    eval_t score;
    int n = 0;

    if (depth == 0) {
        //return quiesce(p, alpha, beta);           /* leaf node */
        moves_gen_all_nomoves(pos);
        score = eval(pos) * color;
        return score;
    }

    moves_gen_eval_sort(pos);

    //moves_print(pos, M_PR_EVAL);
    list_for_each_entry(move, &pos->moves[pos->turn], list) {
        log(1, "%.*s", 5 - depth, "                ");
        newpos = move->pos;
        log_f(1, "depth=%d eval=%d move=", depth, move->eval);
        move_print(0, move, M_PR_EVAL);
        log(1, "\n");
        if (!n++) {                               /* first child */
            score = -pvs(newpos, depth - 1, -beta, -alpha, -color);
        } else {
            /* search with a null window */
            score = -pvs(newpos, depth - 1, -alpha - 1, -alpha, -color);
            if (score > alpha && score < beta) {
                /* if failed high, do a full re-search */
                score = -pvs(newpos, depth - 1, -beta, -score, -color);
            }
        }
        if (score > alpha) {
            alpha = score;
            pos->bestmove = move;
        }
        move_undo(newpos, move);
        move->pos = NULL;
        pos->node_count += newpos->node_count;
        move->negamax = score;
        // alpha = max(alpha, score);
        if (alpha > beta) {                       /* beta cut-off */
            break;
        }
    }

    return alpha;
}

/*
 * int negascout (pos_t *pos, int depth, int alpha, int beta )
 * {                     /\* compute minimax value of position p *\/
 *     move_t *move;
 *     pos_t *newpos;
 *     eval_t best = EVAL_MIN, score;
 *
 *     int a, b, t, i;
 *
 *     if (depth == 0) {
 *         //return quiesce(p, alpha, beta);           /\* leaf node *\/
 *         moves_gen_all_nomoves(pos);
 *         score = eval(pos) * color;
 *         return score;
 *     }
 *     moves_gen_all(pos);
 *     a = alpha;
 *     b = beta;
 *     list_for_each_entry(move, &pos->moves[pos->turn], list) {
 *         log(1, "%.*s", 5 - depth, "                ");
 *         newpos = move_do(pos, move);
 * //    for ( i = 1; i <= w; i++ ) {
 *         t = -negascout (newpos, depth - 1, -b, -alpha);
 *         if ( (t > a) && (t < beta) && (i > 1) )
 *             t = -NegaScout ( p_i, -beta, -alpha ); /\* re-search *\/
 *         alpha = max( alpha, t );
 *         if ( alpha >= beta )
 *             return alpha;                            /\* cut-off *\/
 *         b = alpha + 1;                  /\* set new null window *\/
 *     }
 *     return alpha;
 * }
 */

/*
 * int quiesce(pos_t *pos, int alpha, int beta)
 * {
 *     int stand_pat = eval(pos);
 *
 *     if( stand_pat >= beta )
 *         return beta;
 *     if( alpha < stand_pat )
 *         alpha = stand_pat;
 *
 *     /\*
 *      * until( every_capture_has_been_examined )  {
 *      *     MakeCapture();
 *      *     score = -Quiesce( -beta, -alpha );
 *      *     TakeBackMove();
 *      *
 *      *     if( score >= beta )
 *      *         return beta;
 *      *     if( score > alpha )
 *      *        alpha = score;
 *      * }
 *      *\/
 *     return alpha;
 * }
 */

/**
 * ab_negamax() - search position negamax with alpha-beta cutoff.
 * @pos: &position to search
 * @depth: Wanted depth.
 * @color: 1 for white, -1 for black.
 *
 * Calculate the negamax value of @pos, with alpha-beta pruning.
 *
 * @return: The @pos negamax evaluation.
 */
/*int ab_negamax(pos_t *pos, int alpha, int beta, int depth)
{
    move_t *move;
    pos_t *newpos;
    eval_t best = EVAL_MIN, score;

    if(depth == 0) {
       //return quiesce( alpha, beta );
        moves_gen_all_nomoves(pos);
       score = eval(pos) * color;
       return score;
    }
   for ( all moves)  {
      score = -alphaBeta( -beta, -alpha, depthleft - 1 );
      if( score >= beta )
         return beta;   //  fail hard beta-cutoff
      if( score > alpha )
         alpha = score; // alpha acts like max in MiniMax
   }
   return alpha;
}
*/
