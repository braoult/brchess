/* move-gen.c - move generation
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

#include <stdio.h>

#include "bitops.h"
#include "bug.h"

#include "chessdefs.h"
#include "board.h"
#include "bitboard.h"
#include "piece.h"
#include "position.h"
#include "move.h"
#include "hyperbola-quintessence.h"
#include "attack.h"
#include "move-gen.h"


/**
 * pseudo_is_legal() - check if a move is legal.
 * @pos:  position
 * @move: move_t
 *
 * We check all possible invalid moves:
 * (1) King:
 *  - K moves to a controlled square
 * (1) Castling:
 *  - K passes a controlled square   - already done in pseudomove gen
 *
 *
 * (3) En-passant:
 *  - pinned taking pawn, done in (3)
 *  - taking and taken pawn on same rank than king, discovered check on rank
 *
 * (3) Pinned pieces:
 *  - if destination square quits "pinned line"
 *
 * @return: true if move is valid, false otherwise.
 */
bool pseudo_is_legal(pos_t *pos, move_t move)
{
    int from = move_from(move);
    int to = move_to(move);
    int us = pos->turn;
    int them = OPPONENT(us);
    int piece = pos->board[from];

    /* (1) - castling, skipped */

    /* (2) - King */
    if (piece == KING) {
        /* For castling, we already intermediate and destination squares
         * attacks in pseudo move generation, so we only care destination
         * square here.
         */
        return sq_attackers(pos, to, them) ? false: true;
    }

    /* 3 - en-passant */


    /*
     * if (bb_test(pins, from) && !bb_test(Ray[king][from], to))
     *         return false;
     * // En-passant special case: also illegal if self-check through the en-passant captured pawn
     * if (to == pos->epSquare && piece == PAWN) {
     *     const int us = pos->turn, them = opposite(us);
     *     bitboard_t occ = pos_pieces(pos);
     *     bb_clear(&occ, from);
     *     bb_set(&occ, to);
     *     bb_clear(&occ, to + push_inc(them));
     *     return !(bb_rook_attacks(king, occ) & pos_pieces_cpp(pos, them, ROOK, QUEEN)) &&
     *         !(bb_bishop_attacks(king, occ) & pos_pieces_cpp(pos, them, BISHOP, QUEEN));
     * } else
     *     return true;
     */
    return true;
}

/**
 * gen_all_pseudomoves() - generate all pseudo moves
 * @pos: position
 *
 * Generate all @pos pseudo moves for player-to-move.
 * The @pos->moves table is filled with the moves.
 *
 * Only a few validity checks are done here (i.e. moves are not generated):
 *  - castling, if king is in check
 *  - castling, if king passes an enemy-controlled square (not final square).
 * When immediately known, a few move flags are also applied in these cases:
 *  - castling: M_CASTLE_{K,Q}
 *  - pawn capture (incl. en-passant): M_CAPTURE
 *  - promotion: M_PROMOTION
 *  - promotion and capture
 *
 * TODO: move code to specific functions (especially castling, pawn push/capture)
 *
 * @Return: The total number of moves.
 */
int gen_all_pseudomoves(pos_t *pos)
{
    color_t us = pos->turn, them = OPPONENT(us);

    bitboard_t my_pieces     = pos->bb[us][ALL_PIECES];
    bitboard_t not_my_pieces = ~my_pieces;
    bitboard_t enemy_pieces  = pos->bb[them][ALL_PIECES];

    bitboard_t occ           = my_pieces | enemy_pieces;
    bitboard_t empty         = ~occ;

    bitboard_t movebits, from_pawns;
    bitboard_t tmp1, tmp2;

    move_t *moves            = pos->moves.move;
    int nmoves               = pos->moves.nmoves;

    int from, to;

    /* king - MUST BE FIRST ! */
    from = pos->king[us];
    movebits = bb_king_moves(not_my_pieces, from);
    bit_for_each64(to, tmp2, movebits) {
        moves[nmoves++] = move_make(from, to);
    }
    if (popcount64(pos->checkers) > 1)            /* double check, we stop here */
        return (pos->moves.nmoves = nmoves);

    /* sliding pieces */
    bit_for_each64(from, tmp1, pos->bb[us][BISHOP]) {
        movebits = hyperbola_bishop_moves(occ, from) & not_my_pieces;
        bit_for_each64(to, tmp2, movebits) {
            moves[nmoves++] = move_make(from, to);
        }
    }
    bit_for_each64(from, tmp1, pos->bb[us][ROOK]) {
        // printf("rook=%d/%s\n", from, sq_to_string(from));
        movebits = hyperbola_rook_moves(occ, from) & not_my_pieces;
        bit_for_each64(to, tmp2, movebits) {
            moves[nmoves++] = move_make(from, to);
        }
    }
    /* TODO: remove this one, after movegen is validated */
    bit_for_each64(from, tmp1, pos->bb[us][QUEEN]) {
        movebits = hyperbola_queen_moves(occ, from) & not_my_pieces;
        bit_for_each64(to, tmp2, movebits) {
            moves[nmoves++] = move_make(from, to);
        }
    }

    /* knight */
    bit_for_each64(from, tmp1, pos->bb[us][KNIGHT]) {
        movebits = bb_knight_moves(not_my_pieces, from);
        bit_for_each64(to, tmp2, movebits) {
            moves[nmoves++] = move_make(from, to);
        }
    }

    /* pawn: relative rank and files */
    bitboard_t rel_rank7 = us == WHITE ? RANK_7bb : RANK_2bb;
    bitboard_t rel_rank3 = us == WHITE ? RANK_3bb : RANK_6bb;
    //printf("r7_o = %016lx\nr7_n = %016lx\nsize=%lu\n", rel_rank7, BB_REL_RANK(RANK_7, me),
    //       sizeof(RANK_3bb));
    //printf("r3_o = %016lx\nr3_n = %016lx\nsize=%lu\n", rel_rank3, BB_REL_RANK(RANK_3, me),
    //       sizeof(RANK_3bb));
    //printf("fc_o = %016lx\nfc_n = %016lx\nsize=%lu\n", FILE_Cbb, BB_REL_FILE(FILE_C, me),
    //       sizeof(RANK_3bb));
    //bitboard_t rel_filea = (me == WHITE ? FILE_Abb : FILE_Hbb);
    //bitboard_t rel_fileh = (me == WHITE ? FILE_Hbb : FILE_Abb);
    int en_passant = pos->en_passant == SQUARE_NONE? 0: pos->en_passant;
    bitboard_t enemy_avail = bb_sq[en_passant] | enemy_pieces;

    /* pawn: ranks 2-6 push 1 and 2 squares */
    movebits = pawn_shift_up(pos->bb[us][PAWN] & ~rel_rank7, us) & empty;
    bit_for_each64(to, tmp1, movebits) {
        from = pawn_push_up(to, them);              /* reverse push */
        //printf("push %d->%d %s->%s", from, to, sq_to_string(from), sq_to_string(to));
        moves[nmoves++] = move_make(from, to);
    }
    /* possible second push */
    movebits = pawn_shift_up(movebits & rel_rank3, us) & empty;
    bit_for_each64(to, tmp1, movebits) {
        from = pawn_push_up(pawn_push_up(to, them), them);
        moves[nmoves++] = move_make(from, to);
    }

    /* pawn: ranks 2-6 captures left, including en-passant */
    from_pawns = pos->bb[us][PAWN] & ~rel_rank7; // & ~rel_filea;
    movebits = pawn_shift_upleft(from_pawns, us) & enemy_avail;
    bit_for_each64(to, tmp1, movebits) {
        from = pawn_push_upleft(to, them);         /* reverse capture */
        moves[nmoves++] = move_make_capture(from, to);
    }
    /* pawn: ranks 2-6 captures right, including en-passant */
    from_pawns = pos->bb[us][PAWN] & ~rel_rank7; // & ~rel_fileh;
    movebits = pawn_shift_upright(from_pawns, us) & enemy_avail;
    bit_for_each64(to, tmp1, movebits) {
        from = pawn_push_upright(to, them);
        moves[nmoves++] = move_make_capture(from, to);
    }

    /* pawn: rank 7 push */
    movebits = pawn_shift_up(pos->bb[us][PAWN] & rel_rank7, us) & empty;
    bit_for_each64(to, tmp1, movebits) {
        from = pawn_push_up(to, them);              /* reverse push */
        moves[nmoves++] = move_make_promote(from, to, QUEEN);
        moves[nmoves++] = move_make_promote(from, to, ROOK);
        moves[nmoves++] = move_make_promote(from, to, BISHOP);
        moves[nmoves++] = move_make_promote(from, to, KNIGHT);
    }
    /* pawn promotion: rank 7 captures left */
    from_pawns = pos->bb[us][PAWN] & rel_rank7; // & ~rel_filea;
    movebits = pawn_shift_upleft(from_pawns, us) & enemy_avail;
    bit_for_each64(to, tmp1, movebits) {
        from = pawn_push_upleft(to, them);         /* reverse capture */
        moves[nmoves++] = move_make_promote_capture(from, to, QUEEN);
        moves[nmoves++] = move_make_promote_capture(from, to, ROOK);
        moves[nmoves++] = move_make_promote_capture(from, to, BISHOP);
        moves[nmoves++] = move_make_promote_capture(from, to, KNIGHT);
    }
    /* pawn: rank 7 captures right */
    from_pawns = pos->bb[us][PAWN] & rel_rank7; // & ~rel_fileh;
    movebits = pawn_shift_upright(from_pawns, us) & enemy_pieces;
    bit_for_each64(to, tmp1, movebits) {
        from = pawn_push_upright(to, them);         /* reverse capture */
        moves[nmoves++] = move_make_promote_capture(from, to, QUEEN);
        moves[nmoves++] = move_make_promote_capture(from, to, ROOK);
        moves[nmoves++] = move_make_promote_capture(from, to, BISHOP);
        moves[nmoves++] = move_make_promote_capture(from, to, KNIGHT);
    }

    /* castle - Attention ! Castling flags are assumed correct
     */
    if (!pos->checkers) {
        bitboard_t rel_rank1 = BB_REL_RANK(RANK_1, us);
        from = pos->king[us];
        square_t from_square[2] = { E1, E8 };     /* verify king is on E1/E8 */
        bug_on(CAN_CASTLE(pos->castle, us) && from != from_square[us]);
        /* For castle, we check the opponent attacks on squares between from and to.
         * To square attack check will be done in gen_is_legal.
         */
        if (CAN_OO(pos->castle, us)) {
            bitboard_t occmask = rel_rank1 & (FILE_Fbb | FILE_Gbb);
            if (!(occ & occmask) &&
                !sq_attackers(pos, from+1, them)) { /* f1/f8 */
                moves[nmoves++] = move_make_flags(from, from + 2, M_CASTLE_K);
            }
        }
        if (CAN_OOO(pos->castle, us)) {
            bitboard_t occmask = rel_rank1 & (FILE_Bbb | FILE_Cbb | FILE_Dbb);
            if (!(occ & occmask) &&
                !sq_attackers(pos, from-1, them)) { /* d1/d8 */
                moves[nmoves++] = move_make_flags(from, from - 2, M_CASTLE_Q);
            }
        }
    }
    /* TODO
     * DONE. pawn ranks 2-6 advance (1 push, + 2 squares for rank 2)
     * DONE. pawns rank 7 advance + promotions
     * DONE. pawns ranks 2-6 captures, left and right
     * DONE. pawns en-passant (with capture)
     * DONE. pawns rank 7 capture + promotion
     * DONE. castle
     *
     * add function per piece, and type, for easier debug
     *
     */
    return (pos->moves.nmoves = nmoves);
}
