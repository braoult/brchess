/* search.c - perft + search.
 *
 * Copyright (C) 2023-2024 Bruno Raoult ("br")
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

#include <brlib.h>

#include "position.h"
#include "move-gen.h"
#include "move-do.h"
#include "search.h"
#include "attack.h"

/**
 * perft() - Perform perft on position
 * @pos:    &position to search
 * @depth:  Wanted depth.
 * @ply:    current perft depth level (root = 1)
 * @output: output total for 1st level moves.
 *
 * Run perft on a position. This function displays the available moves at @depth
 * level for each possible first move, and the total of moves.
 *
 * This version uses the algorithm:
 *    if last depth
 *      return 1;
 *    gen legal moves
 *    loop for legal move
 *      do-move
 *      perft (depth -1)
 *      undo-move
 *
 * @return: total moves found at @depth level.
 */
u64 perft(pos_t *pos, int depth, int ply, bool output)
{
    u64 subnodes, nodes = 0;
    movelist_t movelist;
    move_t *move, *last;
    state_t state;
#   ifdef PERFT_MOVE_HISTORY
    static movelist_t stack;
    if (ply == 1)
        stack.nmoves = 0;
#   endif

    pos_set_checkers_pinners_blockers(pos);

    pos_legal(pos, pos_gen_pseudo(pos, &movelist));
    last = movelist.move + movelist.nmoves;
    for (move = movelist.move; move < last; ++move) {
        if (depth == 1) {
            nodes++;
        } else {
            move_do(pos, *move, &state);
#           ifdef PERFT_MOVE_HISTORY
            stack.move[stack.nmoves++] = *move;
#           endif
            if (depth == 2) {
                movelist_t movelist2;
                pos_set_checkers_pinners_blockers(pos);
                subnodes = pos_legal(pos, pos_gen_pseudo(pos, &movelist2))->nmoves;
            } else if (ply >= 3) {
                hentry_t *entry = tt_probe_perft(pos->key, depth);
                if (entry != TT_MISS) {
                    subnodes = HASH_PERFT_VAL(entry->data);
                } else {
                    subnodes = perft(pos, depth - 1, ply + 1, output);
                    tt_store_perft(pos->key, depth, subnodes);
                }
            } else {
                subnodes = perft(pos, depth - 1, ply + 1, output);
            }
            if (output && ply == 1) {
                char movestr[8];
                printf("%s: %lu\n", move_to_str(movestr, *move, 0), subnodes);
            }
            nodes += subnodes;
            move_undo(pos, *move, &state);
#           ifdef PERFT_MOVE_HISTORY
            stack.nmoves--;
#           endif
        }
    }

    if (output && ply == 1)
        printf("Total: %lu\n", nodes);
    return nodes;
}

/**
 * perft_alt() - Perform perft on position, experimental version.
 * @pos:    &position to search
 * @depth:  Wanted depth.
 * @ply:    current perft depth level (root = 1)
 * @output: output total for 1st level moves.
 *
 * Run perft on a position. This function displays the available moves at @depth
 * level for each possible first move, and the total of moves.
 *
 * @return: total moves found at @depth level.
 */
u64 perft_alt(pos_t *pos, int depth, int ply, bool output)
{
    u64 subnodes, nodes = 0;
    movelist_t movelist;
    move_t *move, *last;
    state_t state;

    pos_set_checkers_pinners_blockers(pos);

    pos_legal(pos, pos_gen_pseudo(pos, &movelist));
    last = movelist.move + movelist.nmoves;
    for (move = movelist.move; move < last; ++move) {
        if (depth == 1) {
            nodes++;
        } else {
            move_do_alt(pos, *move, &state);
            if (depth == 2) {
                movelist_t movelist2;
                pos_set_checkers_pinners_blockers(pos);
                subnodes = pos_legal(pos, pos_gen_pseudo(pos, &movelist2))->nmoves;
            } else {
                subnodes = perft_alt(pos, depth - 1, ply + 1, output);
            }
            if (output && ply == 1) {
                char movestr[8];
                printf("%s: %lu\n", move_to_str(movestr, *move, 0), subnodes);
            }
            nodes += subnodes;
            move_undo_alt(pos, *move, &state);
        }
    }

    if (output && ply == 1)
        printf("Total: %lu\n", nodes);
    return nodes;
}

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
/*
 * eval_t negamax(pos_t *pos, int depth, int color)
 * {
 *     move_t *move;
 *     pos_t *newpos;
 *     eval_t best = EVAL_MIN, score;
 *
 *     pos->node_count++;
 *     if (depth == 0) {
 *         moves_gen_all_nomoves(pos);
 *         score = eval(pos) * color;
 *         return score;
 *     }
 *     moves_gen_all(pos);
 *     list_for_each_entry(move, &pos->moves[pos->turn], list) {
 *         newpos = move_do(pos, move);
 *         score = -negamax(newpos, depth - 1, -color);
 *         pos->node_count += newpos->node_count;
 *         move->negamax = score;
 *         if (score > best) {
 *             best = score;
 *             pos->bestmove = move;
 *         }
 *         move_undo(newpos, move);
 *     }
 *     return best;
 * }
 */


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
 * Moves list should be first generated and evaluated/sorted.
 *
 * @return: The @pos PVS evaluation.
 */
/*
 * eval_t pvs(pos_t *pos, int depth, int alpha, int beta, int color)
 * {
 *     move_t *move;
 *     pos_t *newpos;
 *     eval_t score = EVAL_INVALID;
 *     bool firstchild = true;
 *
 *     pos->node_count++;
 *
 *     if (depth == 0) {
 *         //return quiesce(p, alpha, beta);           /\* leaf node *\/
 *         moves_gen_all_nomoves(pos);
 *         score = eval(pos) * color;
 *         log_f(2, "Terminal: depth=%d ", depth);
 *         log_f(2, "score=%d alpha=%d beta=%d\n", score, alpha, beta);
 *         return score;
 *     }
 *
 *     moves_gen_all(pos);
 *     //moves_print(pos, M_PR_EVAL);
 *     /\* do the full search for first child *\/
 *     //move = list_first_entry_or_null(&pos->moves[pos->turn], move_t, list);
 *
 *     list_for_each_entry(move, &pos->moves[pos->turn], list) {
 *         newpos = move_do(pos, move);
 *         log(2, "%.*s", 5 - depth, "                ");
 *         if (firstchild) {                               /\* first child *\/
 *             score = -pvs(newpos, depth - 1, -beta, -alpha, -color);
 *             log_f(2, "First child depth=%d move=", depth);
 *             //move_print(0, move, 0);
 *             log(2, "score=%d alpha=%d beta=%d\n", score, alpha, beta);
 *             pos->bestmove = move;
 *         } else {
 *             /\* search with a null window *\/
 *             score = -pvs(newpos, depth - 1, -alpha - 1, -alpha, -color);
 *             log_f(2, "Other child depth=%d move=", depth);
 *             //move_print(0, move, 0);
 *             log_f(2, "score=%d alpha=%d beta=%d ", score, alpha, beta);
 *             /\* for fail-soft:  if (score > alpha && score < beta) *\/
 *             if (score > alpha) {
 *                 /\* if failed high, do a full re-search *\/
 *                 log_f(2, "doing full search.");
 *                 score = -pvs(newpos, depth - 1, -beta, -alpha, -color);
 *             }
 *             log(2, "\n");
 *         }
 *         pos->node_count += newpos->node_count;
 *         move_undo(newpos, move);
 *         if (score >= beta) {                      /\* fail-hard hard beta cut-off *\/
 *             log(2, "%.*s", 5 - depth, "                ");
 *             log_f(2, "depth=%d score=%d alpha=%d beta=%d beta cut-off.\n",
 *                   depth, score, alpha, beta);
 *             return beta;
 *         }
 *         if (score > alpha) {
 *             log(2, "%.*s", 5 - depth, "                ");
 *             log_f(2, "depth=%d setting new alpha from %d to %d\n",
 *                   depth, alpha, score);
 *             alpha = score;
 *             pos->bestmove = move;
 *         }
 *         move->pos = NULL;
 *         move->negamax = score;
 *         firstchild = false;
 *     }
 *
 *     return alpha;
 * }
 */

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
