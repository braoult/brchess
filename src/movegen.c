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

/**
 * gen_pawn_push() - generate pawn push
 * @pos: position
 *
 * Generate all pseudo pawn pushes.
 */
//int gen_pawn_push(pos_t *pos, bitboard_t occ)
 //{


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
    color_t me = pos->turn, enemy = OPPONENT(me);
    bitboard_t my_pieces = pos->bb[me][ALL_PIECES], not_my_pieces = ~my_pieces,
        enemy_pieces = pos->bb[enemy][ALL_PIECES];
    bitboard_t occ = my_pieces | enemy_pieces, empty = ~occ;
    bitboard_t movebits, from_pawns;

    int tmp1, tmp2, from, to;
    movelist_t moves = pos->moves;

    /* sliding pieces */
    bit_for_each64(from, tmp1, pos->bb[me][BISHOP]) {
        movebits = hyperbola_bishop_moves(occ, from) & not_my_pieces;
        bit_for_each64(to, tmp2, movebits) {
            moves.move[moves.nmoves++] = move_make(from, to);
        }

    }
    bit_for_each64(from, tmp1, pos->bb[me][ROOK]) {
        movebits = hyperbola_rook_moves(occ, from) & not_my_pieces;
        bit_for_each64(to, tmp2, movebits) {
            moves.move[moves.nmoves++] = move_make(from, to);
        }
    }
    /* TODO: remove this one */
    bit_for_each64(from, tmp1, pos->bb[me][QUEEN]) {
        movebits = hyperbola_queen_moves(occ, from) & not_my_pieces;
        bit_for_each64(to, tmp2, movebits) {
            moves.move[moves.nmoves++] = move_make(from, to);
        }
    }

    /* knight */
    bit_for_each64(from, tmp1, pos->bb[me][KNIGHT]) {
        movebits = bb_knight_moves(occ, from) & not_my_pieces;
        bit_for_each64(to, tmp2, movebits) {
            moves.move[moves.nmoves++] = move_make(from, to);
        }
    }

    /* king */
    movebits = bb_king_moves(occ, pos->king[me]) & not_my_pieces;
    bit_for_each64(to, tmp2, movebits) {
        moves.move[moves.nmoves++] = move_make(from, to);
    }

    /* pawn: relative rank and files */
    bitboard_t rel_rank7 = (me == WHITE ? RANK_7bb : RANK_2bb);
    bitboard_t rel_rank3 = (me == WHITE ? RANK_3bb : RANK_6bb);
    bitboard_t rel_filea = (me == WHITE ? FILE_Abb : FILE_Hbb);
    bitboard_t rel_fileh = (me == WHITE ? FILE_Hbb : FILE_Abb);
    int en_passant = pos->en_passant == SQUARE_NONE? 0: pos->en_passant;
    bitboard_t enemy_avail = bb_sq[en_passant] | enemy_pieces;

    /* pawn: ranks 2-6 push 1 and 2 squares */
    movebits = pawn_push(pos->bb[me][PAWN] & ~rel_rank7, me) & empty;
    bit_for_each64(to, tmp1, movebits) {
        from = pawn_push(to, enemy);              /* reverse push */
        moves.move[moves.nmoves++] = move_make(from, to);
    }
    movebits = pawn_push(movebits & rel_rank3, me) & empty;
    bit_for_each64(to, tmp1, movebits) {
        from = pawn_push(pawn_push(to, enemy), enemy);
        moves.move[moves.nmoves++] = move_make(from, to);
    }
    /* pawn: rank 7 push */
    movebits = pawn_push(pos->bb[me][PAWN] & rel_rank7, me) & empty;
    bit_for_each64(to, tmp1, movebits) {
        from = pawn_push(to, enemy);              /* reverse push */
        moves.move[moves.nmoves++] = move_make_promote(from, to, QUEEN);
        moves.move[moves.nmoves++] = move_make_promote(from, to, ROOK);
        moves.move[moves.nmoves++] = move_make_promote(from, to, BISHOP);
        moves.move[moves.nmoves++] = move_make_promote(from, to, KNIGHT);
    }

    /* pawn: ranks 2-6 captures left, including en-passant */
    from_pawns = pos->bb[me][PAWN] & ~rel_rank7 & ~rel_filea;
    movebits = pawn_take_left(from_pawns, me) & enemy_avail;
    bit_for_each64(to, tmp1, movebits) {
        from = pawn_take_left(to, enemy);         /* reverse capture */
        moves.move[moves.nmoves++] = move_make(from, to);
    }
    /* pawn: rank 7 captures left */
    from_pawns = pos->bb[me][PAWN] & rel_rank7 & ~rel_filea;
    movebits = pawn_take_left(from_pawns, me) & enemy_avail;
    bit_for_each64(to, tmp1, movebits) {
        from = pawn_take_left(to, enemy);         /* reverse capture */
        moves.move[moves.nmoves++] = move_make_promote(from, to, QUEEN);
        moves.move[moves.nmoves++] = move_make_promote(from, to, ROOK);
        moves.move[moves.nmoves++] = move_make_promote(from, to, BISHOP);
        moves.move[moves.nmoves++] = move_make_promote(from, to, KNIGHT);
    }

    /* pawn: ranks 2-6 captures right, including en-passant */
    from_pawns = pos->bb[me][PAWN] & ~rel_rank7 & ~rel_fileh;
    movebits = pawn_take_right(from_pawns, me) & enemy_avail;
    bit_for_each64(to, tmp1, movebits) {
        from = pawn_take_right(to, enemy);
        moves.move[moves.nmoves++] = move_make(from, to);
    }
    /* pawn: rank 7 captures right */
    from_pawns = pos->bb[me][PAWN] & rel_rank7 & ~rel_fileh;
    movebits = pawn_take_right(from_pawns, me) & enemy_pieces;
    bit_for_each64(to, tmp1, movebits) {
        from = pawn_take_right(to, enemy);         /* reverse capture */
        moves.move[moves.nmoves++] = move_make_promote(from, to, QUEEN);
        moves.move[moves.nmoves++] = move_make_promote(from, to, ROOK);
        moves.move[moves.nmoves++] = move_make_promote(from, to, BISHOP);
        moves.move[moves.nmoves++] = move_make_promote(from, to, KNIGHT);
    }

    /* TODO
     * DONE. pawn ranks 2-6 advance (1 push, + 2 squares for rank 2)
     * DONE. pawns rank 7 advance + promotions
     * DONE. pawns ranks 2-6 captures, left and right
     * DONE. pawns en-passant (with capture)
     * DONE. pawns rank 7 capture + promotion
     * castle
     *
     * add function per piece, and type, for easier debug
     *
     */
    return moves.nmoves;
}
