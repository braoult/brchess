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
 * @move: move_t to verify
 *
 * @return: true if move is valid, false otherwise.
 */
bool pseudo_is_legal(const pos_t *pos, const move_t move)
{
    color_t us = pos->turn, them = OPPONENT(us);
    square_t from = move_from(move), to = move_to(move), king = pos->king[us], sq;
    piece_type_t piece = PIECE(pos->board[from]);
    bitboard_t kingbb = pos->bb[us][KING], tmp;

    /* (1) - King
     * For castling, we already tested intermediate squares attacks
     * in pseudo move generation, so we only care destination square here.
     * Attention: We need to exclude king from occupation bitboard !
     */
    if (piece == KING) {
        bitboard_t occ = pos_occ(pos) ^ kingbb;
        return !sq_attackers(pos, occ, to, them);
    }

    /* (2) - King is in check
     * Double-check is already handled, as only K moves were generated.
     * Here, allowed moves are only on King-attacker line, including
     * attacker.We can move a piece on
     * in pseudo move generation, so we only care destination square here.
     */
    if (pos->checkers) {
        if (popcount64(pos->checkers) == 1) {     /* one checker */
            square_t checker = ctz64(pos->checkers);
            bitboard_t between = bb_between[king][checker];
            return mask(to) & between;
        }
        return false;                             /* double check */
    }

    /* (3) - pinned pieces
     * We verify here that pinned piece P stays on line King-P.
     */
    if (mask(from) & pos->blockers & pos->bb[us][ALL_PIECES]) {
        bitboard_t line = bb_line[from][king];
        return line & mask(to);                   /* to is not on pin line */
    }

    /* (4) - En-passant
     * We only care the situation where our King and enemy R/Q are on
     * 5th relative rank. To do so, we create an occupation bb without
     * the 2 pawns.
     */
    if (is_enpassant(move)) {
         /* from rank bitboard */
        bitboard_t rank5 = bb_sqrank[from];
        /* enemy rooks/queens on from rank */
        bitboard_t rooks = (pos->bb[them][ROOK] | pos->bb[them][QUEEN]) & rank5;

        if ((kingbb & rank5) && rooks) { /* K and enemy R/Q on rank */
            /* captured pawn square (beside from square) */
            square_t captured = sq_make(sq_file(pos->en_passant), sq_rank(from));
            /* occupation bitboard without the two "disappearing" pawns */
            bitboard_t occ = pos_occ(pos) ^ mask(from) ^ mask(captured);

            bit_for_each64(sq, tmp, rooks)        /* check all rooks/queens */
                if (hyperbola_rank_moves(occ, sq) & kingbb)
                    return false;
        }
        return true;
    }
    return true;
}

/**
 * pos_next_legal() - get next legal move in position.
 * @pos:  position
 * @start:  &int, starting position in move list
 *
 * Get next valid move in @pos move list, from move @start, or MOVE_NONE.
 * @start is set to next non-checked move in pseudo-legal list.
 * Position pseudo-legal moves must be already calculated before calling this function.
 *
 * @return: move, or -1 if no move.
 */
move_t pos_next_legal(const pos_t *pos, int *start)
{
    const int nmoves = pos->moves.nmoves;
    const move_t *moves = pos->moves.move;
    move_t move;

    while (*start < nmoves) {
        if (pseudo_is_legal(pos, (move = moves[(*start)++] )))
            return  move;
    }
    return MOVE_NONE;
}

/**
 * pos_all_legal() - get the list of legal moves from pseudo-legal.
 * @pos:  position
 * @dest: destination &movelist_t
 *
 * The pseudo-legal moves must be already calculated before calling this function.
 * No check is done on @dest limits.
 *
 * @Return: @dest
 */
movelist_t *pos_all_legal(const pos_t *pos, movelist_t *dest)
{
    int tmp = dest->nmoves = 0;
    move_t move;
    //int tmp = 0;

    while ((move = pos_next_legal(pos, &tmp)) != MOVE_NONE)
        dest->move[dest->nmoves++] = move;
    return dest;
}

/**
 * pos_gen_pseudomoves() - generate position pseudo-legal moves
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
 *                         en-passant: M_EN_PASSANT
 *  - promotion: M_PROMOTION
 *  - promotion and capture
 *
 * TODO: move code to specific functions (especially castling, pawn push/capture)
 *
 * @Return: The total number of moves.
 */
int pos_gen_pseudomoves(pos_t *pos)
{
    color_t us               = pos->turn;
    color_t them             = OPPONENT(us);

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
    bit_for_each64(to, tmp1, movebits) {
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

    /* pawn: ranks 2-6 captures left */
    from_pawns = pos->bb[us][PAWN] & ~rel_rank7; // & ~rel_filea;
    movebits = pawn_shift_upleft(from_pawns, us) & enemy_pieces;
    bit_for_each64(to, tmp1, movebits) {
        from = pawn_push_upleft(to, them);         /* reverse capture */
        moves[nmoves++] = move_make_capture(from, to);
    }
    /* pawn: ranks 2-6 captures right */
    from_pawns = pos->bb[us][PAWN] & ~rel_rank7; // & ~rel_fileh;
    movebits = pawn_shift_upright(from_pawns, us) & enemy_pieces;
    bit_for_each64(to, tmp1, movebits) {
        from = pawn_push_upright(to, them);
        moves[nmoves++] = move_make_capture(from, to);
    }

    /* pawn: en-passant */
    if ((to = pos->en_passant) != SQUARE_NONE) {
        movebits = mask(to);
        from_pawns = pos->bb[us][PAWN]
            & (pawn_shift_upleft(movebits, them) | pawn_shift_upright(movebits, them));
        bit_for_each64(from, tmp1, from_pawns) {
            moves[nmoves++] = move_make_enpassant(from, to);
        }
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
    movebits = pawn_shift_upleft(from_pawns, us) & enemy_pieces;
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
                !sq_attackers(pos, occ, from+1, them)) { /* f1/f8 */
                moves[nmoves++] = move_make_flags(from, from + 2, M_CASTLE_K);
            }
        }
        if (CAN_OOO(pos->castle, us)) {
            bitboard_t occmask = rel_rank1 & (FILE_Bbb | FILE_Cbb | FILE_Dbb);
            if (!(occ & occmask) &&
                !sq_attackers(pos, occ, from-1, them)) { /* d1/d8 */
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
