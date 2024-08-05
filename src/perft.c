/* perft.c - perft functions.
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

#include "perft.h"
#include "move-gen.h"
#include "move-do.h"

/**
 * do_perft() - perft engine
 * @pos:    &position to search
 * @depth:  Wanted depth.
 *
 * Run perft on a position.
 *
 * This version uses the algorithm:
 *    gen legal moves
 *    if last depth
 *      return number of legal move
 *    loop for legal move
 *      do-move
 *      if depth == 2
 *
 *      do_perft (depth -1)
 *      undo-move
 *
 * @return: total moves found at @depth level.
 */
static u64 do_perft(pos_t *pos, int depth)
{
    u64 nodes = 0;
    movelist_t movelist;
    move_t *move, *last;
    state_t state;

    pos_set_checkers_pinners_blockers(pos);
    pos_legal(pos, pos_gen_pseudo(pos, &movelist));

    //if (depth == 1)
    //    return movelist.nmoves;

    last = movelist.move + movelist.nmoves;
    switch (depth) {
        case 1:
            /* This case could be removed if 'case 2' is handled in perft()
             */
            return movelist.nmoves;
            break;
        case 2:
            /* For depth 2, we directly calculate the possible legal moves
             * after each possible moves.
             */
            for (move = movelist.move; move < last; ++move) {
                move_do(pos, *move, &state);
                movelist_t movelist2;
                pos_set_checkers_pinners_blockers(pos);
                nodes += pos_legal(pos, pos_gen_pseudo(pos, &movelist2))->nmoves;
                move_undo(pos, *move, &state);
            }
            break;
        default:
            /* Default: Search in TT for same key+depth. Use it if found, create
             * it otherwise.
             */
            for (move = movelist.move; move < last; ++move) {
                move_do(pos, *move, &state);
                hentry_t *entry = tt_probe_perft(pos->key, depth);
                if (entry != TT_MISS) {
                    nodes += HASH_PERFT_VAL(entry->data);
                } else {
                    u64 subnodes = do_perft(pos, depth - 1);
                    tt_store_perft(pos->key, depth, subnodes);
                    nodes += subnodes;
                }
                move_undo(pos, *move, &state);
            }
            //} else {
            //subnodes = do_perft(pos, depth - 1);
    }
        //move_undo(pos, *move, &state);
        //nodes += subnodes;
    //}

    return nodes;
}

/**
 * perft() - Perform perft on position
 * @pos:    &position to search
 * @depth:  Wanted depth.
 * @ply:    current perft depth level (root = 1)
 * @divide: output total for 1st level moves.
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
u64 perft(pos_t *pos, int depth, bool divide)
{
    u64 subnodes = 0, nodes = 0;
    movelist_t movelist;
    move_t *move, *last;
    state_t state;

    pos_set_checkers_pinners_blockers(pos);

    pos_legal(pos, pos_gen_pseudo(pos, &movelist));
    last = movelist.move + movelist.nmoves;
    for (move = movelist.move; move < last; ++move) {
        if (depth == 1) {
            subnodes = 1;
        } else {
            move_do(pos, *move, &state);
            subnodes = do_perft(pos, depth - 1);
            move_undo(pos, *move, &state);
        }
        if (divide) {
            char movestr[8];
            printf("%s: %lu\n", move_to_str(movestr, *move, 0), subnodes);
        }
        nodes += subnodes;
    }
    return nodes;
}

/**
 * perft_alt() - Perform perft on position, experimental version.
 * @pos:    &position to search
 * @depth:  Wanted depth.
 * @ply:    current perft depth level (root = 1)
 * @divide: output total for 1st level moves.
 *
 * Run perft on a position. This function displays the available moves at @depth
 * level for each possible first move, and the total of moves.
 *
 * @return: total moves found at @depth level.
 */
u64 perft_alt(pos_t *pos, int depth, int ply, bool divide)
{
    u64 subnodes = 0, nodes = 0;
    movelist_t movelist;
    move_t *move, *last;
    state_t state;

    pos_set_checkers_pinners_blockers(pos);

    pos_legal(pos, pos_gen_pseudo(pos, &movelist));
    last = movelist.move + movelist.nmoves;
    for (move = movelist.move; move < last; ++move) {
        if (depth == 1) {
            subnodes = 1;
        } else {
            move_do_alt(pos, *move, &state);
            if (depth == 2) {
                movelist_t movelist2;
                pos_set_checkers_pinners_blockers(pos);
                subnodes = pos_legal(pos, pos_gen_pseudo(pos, &movelist2))->nmoves;
            } else {
                subnodes = perft_alt(pos, depth - 1, ply + 1, divide);
            }
            move_undo_alt(pos, *move, &state);
        }
        nodes += subnodes;
        if (ply == 1 && divide) {
            char movestr[8];
            printf("%s: %lu\n", move_to_str(movestr, *move, 0), subnodes);
        }
    }

    return nodes;
}
