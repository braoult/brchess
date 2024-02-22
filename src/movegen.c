/* movegen.c - move generation
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

#include "bitops.h"

#include "bitboard.h"
#include "hyperbola-quintessence.h"
#include "piece.h"
#include "move.h"

//void add_pseudo_move(move_t *pmove, square_t from, square_t to)
//{
//    pmov
//}

/**
 * gen_all_pseudomoves() - generate all pseudo moves
 * @pos: position
 *
 * Generate all moves, no check is done on validity due to castle rules,
 * or check (pinned pieces, etc...).
 */
int gen_all_pseudomoves(pos_t *pos)
{
    int me = pos->turn, other = OPPONENT(me);
    bitboard_t my_pieces = pos->bb[me][ALL_PIECES], notmine = ~my_pieces,
        other_pieces = pos->bb[other][ALL_PIECES],
        occ = my_pieces | other_pieces, movebits;
    int tmp1, tmp2, from, to;
    movelist_t moves = pos->moves;

    /* sliding pieces */
    bit_for_each64(from, tmp1, pos->bb[me][BISHOP]) {
        movebits = hyperbola_bishop_moves(occ, from) & notmine;
        bit_for_each64(to, tmp2, movebits) {
            moves.move[moves.nmoves++] = move_make(from, to);
        }

    }
    bit_for_each64(from, tmp1, pos->bb[me][ROOK]) {
        movebits = hyperbola_rook_moves(occ, from) & notmine;
        bit_for_each64(to, tmp2, movebits) {
            moves.move[moves.nmoves++] = move_make(from, to);
        }
    }
    bit_for_each64(from, tmp1, pos->bb[me][QUEEN]) {
        movebits = hyperbola_queen_moves(occ, from) & notmine;
        bit_for_each64(to, tmp2, movebits) {
            moves.move[moves.nmoves++] = move_make(from, to);
        }
    }

    /* knight */
    bit_for_each64(from, tmp1, pos->bb[me][KNIGHT]) {
        movebits = bb_knight_moves(occ, from) & notmine;
        bit_for_each64(to, tmp2, movebits) {
            moves.move[moves.nmoves++] = move_make(from, to);
        }
    }

    /* king */
    movebits = bb_king_moves(occ, pos->king[me]) & notmine;
    bit_for_each64(to, tmp2, movebits) {
        moves.move[moves.nmoves++] = move_make(from, to);
    }

    /* TODO
     * pawn ranks 2-6 advance (1 push, + 2 squares for rank 2)
     * pawns ranks 2-6 capture
     * pawns rank 7 advance + promotion
     * pawns rank 7 capture + promotion
     * pawns en-passant
     * castle
     */
}
