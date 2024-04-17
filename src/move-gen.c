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
    color_t  us = pos->turn, them = OPPONENT(us);
    square_t from = move_from(move), to = move_to(move);
    square_t king = pos->king[us];
    bitboard_t kingbb = pos->bb[us][KING];
    bitboard_t occ = pos_occ(pos);
    u64 pinned = mask(from) & pos->blockers;
    u64 checkers = pos->checkers;

    /* (1) - Castling & King
     * For castling, we need to check intermediate squares attacks only.
     * Attention: To test if K is in check after moving, we need to exclude
     * king from occupation bitboard (to catch king moving away from checker
     * on same line) !
     */
    if (unlikely(from == king)) {
        if (unlikely(is_castle(move))) {
            square_t dir = to > from? 1: -1;
            if (sq_is_attacked(pos, occ, from + dir, them))
                return false;
        }
        return !sq_is_attacked(pos, occ ^ kingbb, to, them);
    }

    /* (2) - King is in check
     * Double-check is already handled in (1), as only K moves were generated
     * by pseudo legal move generator.
     * Here, allowed dest squares are only on King-checker line, or on checker
     * square.
     * attacker.
     * Special cases:
     *   e.p., legal if the grabbed pawn is giving check
     *   pinned piece: always illegal
     */
    if (checkers) {
        if (pinned)
            return false;
        if (bb_multiple(checkers))
            return false;
        square_t checker = ctz64(checkers);
        if (is_enpassant(move)) {
            return pos->en_passant + sq_up(them) == checker;
        }
        return true;
        //bitboard_t between = bb_between[king][checker] | pos->checkers;
        //return mask(to) & between;
    }

    /* (3) - pinned pieces
     * We verify here that pinned piece P stays on line King-P.
     */
    if (mask(from) & pos->blockers) {
        return bb_line[from][king] & mask(to);    /* is to on pinner line ? */
    }

    /* (4) - En-passant
     * pinned pieces are handled in pinned section.
     * One case not handled anywhere else: when the two "disappearing" pawns
     * would discover a R/Q horizontal check.
     */
    if (is_enpassant(move)) {
        bitboard_t rank5 = us == WHITE? RANK_5bb: RANK_4bb;

        if ((pos->bb[us][KING] & rank5)) {
            bitboard_t exclude = mask(pos->en_passant - sq_up(us)) | mask(from);
            bitboard_t rooks = (pos->bb[them][ROOK] | pos->bb[them][QUEEN]) & rank5;

            while (rooks) {
                square_t rook = bb_next(&rooks);
                if (hyperbola_rank_moves(occ ^ exclude, rook) & kingbb)
                    return false;
            }
        }
    }
    return true;
}

/**
 * pos_next_legal() - get next legal move in position.
 * @pos:  position
 * @movelist: &pseudo-legal movelist
 * @start:  &int, starting position in @movelist
 *
 * Get next valid move in @pos move list, from move @start, or MOVE_NONE.
 * @start is set to next non-checked move in pseudo-legal list.
 * Position pseudo-legal moves must be already calculated before calling this function.
 *
 * @return: move, or MOVE_NONE if no move.
 */
move_t pos_next_legal(const pos_t *pos, movelist_t *movelist, int *start)
{
    const int nmoves = movelist->nmoves;
    const move_t *moves = movelist->move;
    move_t move;

    while (*start < nmoves) {
        if (pseudo_is_legal(pos, (move = moves[(*start)++] )))
            return  move;
    }
    return MOVE_NONE;
}

/**
 * pos_legal_dup() - get legal moves from pseudo-legal ones in new list.
 * @pos:  position
 * @pseudo: &movelist_t pseudo-legal moves list
 * @legal: &movelist_t legal moves
 *
 * The pseudo-legal moves must be already calculated before calling this function.
 * No check is done on @legal limits.
 * This function is similar to pos_legal(), but creates a new list for legal moves.
 * It should only be used for debug purpose, when we want to keep a copy of
 * pseudo-legal moves.
 *
 * @return: @legal
 */
movelist_t *pos_legal_dup(const pos_t *pos, movelist_t *pseudo, movelist_t *legal)
{
    int tmp = legal->nmoves = 0;
    move_t move;

    while ((move = pos_next_legal(pos, pseudo, &tmp)) != MOVE_NONE)
        legal->move[legal->nmoves++] = move;
    return legal;
}

/**
 * pos_legal() - get legal moves from pseudo-legal ones in new list.
 * @pos:  position
 * @list: &movelist_t pseudo-legal moves list
 *
 * The pseudo-legal moves must be already calculated before calling this function.
 * @list is replaced by legal moves.
 *
 * @return: @list
 */
movelist_t *pos_legal(const pos_t *pos, movelist_t *list)
{
    move_t *cur = list->move, *last = list->move + list->nmoves;

    while (cur < last) {
        if (pseudo_is_legal(pos, *cur))
            cur++;
        else {
            *cur = *--last;
        }
    }
    list->nmoves = last - list->move;
    return list;
}

/**
 * gen_pseudo_king() - generate king pseudo-legal moves.
 * @pos: position
 * @movelist: &movelist_t array to store pseudo-moves
 *
 */
static inline __unused move_t *gen_pseudo_king(move_t *moves, square_t from,
                                               bitboard_t mask)
{
    //color_t us    = pos->turn;
    //square_t from = pos->king[us];
    //bitboard_t not_my_pieces     = ~pos->bb[us][ALL_PIECES];
    bitboard_t to_bb = bb_king_moves(mask, from);
    square_t to;
    while (moves) {
        to = bb_next(&to_bb);
        *moves++ = move_make(from, to);
    }
    return moves;
}

/**
 * pos_gen_check_pseudomoves() - generate position pseudo-legal moves when in check
 * @pos: position
 * @movelist: &movelist_t array to store pseudo-moves
 *
 * Generate all @pos pseudo moves for player-to-move.
 * @movelist is filled with the moves.
 *
 * Only a few validity checks are done here (i.e. moves are not generated):
 *  - castling, if king is in check
 *  - castling, if king passes an enemy-controlled square (not final square).
 * When immediately known, a few move flags are also applied in these cases:
 *  - castling: M_CASTLE_{K,Q}
 *  - capture (excl. en-passant): M_CAPTURE
 *  - en-passant: M_EN_PASSANT
 *  - pawn double push: M_DPUSH
 *  - promotion: M_PROMOTION
 *  - promotion and capture
 *
 * TODO: move code to specific functions (especially castling, pawn push/capture)
 *
 * @Return: The total number of moves.
 */

/**
 * gen_pseudo_escape() - generate position pseudo-legal moves when in check
 * @pos: square_t king position
 * @mask: possible destinations to consider
 * @moves: &move_t array to store pseudo-moves
 *
 * Generate all @pos pseudo moves for player-to-move, when in check.
 * @movelist is filled with the moves.
 *
 * If king is doubles-checked, only King moves will be generated.
 * Otherwise, only moves where destination in between King or checker, or checker
 * square.
 *
 */
/*
 * static void __always_inline gen_pseudo_escape(pos_t *pos, movelist_t *movelist)
 * {
 *     color_t us               = pos->turn;
 *     color_t them             = OPPONENT(us);
 *     square_t king            = pos->king[us];
 *
 *     gen_pseudo_king(pos, movelist);
 *
 * }
 */

/**
 * moves_gen_flags() - generate all moves from square to bitboard (with flags).
 * @moves: &move_t array where to store moves
 * @from: square_t piece position
 * @to_bb: destination bitboard
 * @flags: flags to apply
 *
 * Generate (at address @moves) moves from square @from to each square in @to_bb,
 * with flags @flags.
 *
 * @Return: New @moves.
 */
static inline __unused move_t *moves_gen_flags(move_t *moves, square_t from, bitboard_t to_bb,
    __unused move_flags_t flags)
{
    square_t to;
    while(to_bb) {
        to = bb_next(&to_bb);
        *moves++ = move_make_flags(from, to, flags);
    }
    return moves;
}

/**
 * move_gen_promotions() - generate all promotions for given pawn and dest.
 * @moves: &move_t array where to store moves
 * @from: pawn position
 * @to: promotion square
 *
 * Generate  (at address @moves) all promotion (Q/R/B/N) moves on @to for
 * pawn @from.
 *
 * @Return: New @moves.
 */
static inline move_t *move_gen_promotions(move_t *moves, square_t from, square_t to)
{
    for (piece_type_t pt = QUEEN; pt >= KNIGHT; --pt)
        *moves++ = move_make_promote(from, to, pt);
    return moves;
}

/**
 * moves_gen() - generate all moves from square to bitboard.
 * @moves: &move_t array where to store moves
 * @from: square_t piece position
 * @to_bb: destination bitboard
 *
 * Generate (at address @moves) moves from square @from to each square in @to_bb.
 *
 * @Return: New @moves.
 */
static inline move_t *moves_gen(move_t *moves, square_t from, bitboard_t to_bb)
{
    square_t to;
    // bb_print(sq_to_string(from), to_bb);
    while(to_bb) {
        to = bb_next(&to_bb);
        *moves++ = move_make(from, to);
    }
    return moves;
}

/**
 * pos_gen_pseudo() - generate position pseudo-legal moves
 * @pos: position
 * @movelist: &movelist_t array to store pseudo-moves
 *
 * Generate all @pos pseudo moves for player-to-move.
 * @movelist is filled with the moves.
 *
 * Only a few validity checks are done here (i.e. moves are not generated):
 *  - castling, if king is in check
 *  - castling, if king passes an enemy-controlled square (not final square).
 * When immediately known, a few move flags are also applied in these cases:
 *  - castling: M_CASTLE_{K,Q}
 *  - capture (excl. en-passant): M_CAPTURE
 *  - en-passant: M_EN_PASSANT
 *  - pawn double push: M_DPUSH
 *  - promotion: M_PROMOTION
 *  - promotion and capture
 *
 * TODO: move code to specific functions (especially castling, pawn push/capture)
 *
 * @Return: movelist
 */
movelist_t *pos_gen_pseudo(pos_t *pos, movelist_t *movelist)
{
    color_t us               = pos->turn;
    color_t them             = OPPONENT(us);

    bitboard_t my_pieces     = pos->bb[us][ALL_PIECES];
    bitboard_t enemy_pieces  = pos->bb[them][ALL_PIECES];
    bitboard_t dest_squares  = ~my_pieces;
    bitboard_t occ           = my_pieces | enemy_pieces;
    bitboard_t empty         = ~occ;

    bitboard_t from_bb, to_bb;
    bitboard_t tmp_bb;
    move_t *moves            = movelist->move;
    //int *nmoves              = &movelist->nmoves;
    square_t from, to;
    square_t king = pos->king[us];

    //*nmoves = 0;

    /* king - MUST BE FIRST */
    to_bb = bb_king_moves(dest_squares, king);
    moves = moves_gen(moves, king, to_bb);
    //while(to_bb) {
    //    to = bb_next(&to_bb);
    //    *moves++ = move_make(king, to);
    //}

    if (bb_multiple(pos->checkers))               /* double check, we stop here */
        goto finish;

    if (pos->checkers) {
        /* one checker: we limit destination squares to line between
         * checker and king + checker square (think: knight).
         */
        square_t checker = ctz64(pos->checkers);
        dest_squares &= bb_between[king][checker] | pos->checkers;
        enemy_pieces &= dest_squares;
    } else {
        /* no checker: castling moves
         * Attention ! Castling flags are assumed correct
         */
        bitboard_t rel_rank1 = bb_rel_rank(RANK_1, us);
        //square_t from_square[2] = { E1, E8 };     /* verify king is on E1/E8 */
        //bug_on(can_castle(pos->castle, us) && from != from_square[us]);
        /* For castle, we check the opponent attacks on squares between from and to.
         * To square attack check will be done in gen_is_legal.
         */
        if (can_oo(pos->castle, us)) {
            bitboard_t occmask = rel_rank1 & (FILE_Fbb | FILE_Gbb);
            if (!(occ & occmask)) {
                *moves++ = move_make_flags(king, king + 2, M_CASTLE_K);
            }
        }
        if (can_ooo(pos->castle, us)) {
            bitboard_t occmask = rel_rank1 & (FILE_Bbb | FILE_Cbb | FILE_Dbb);
            if (!(occ & occmask)) { // &&
                *moves++ = move_make_flags(king, king - 2, M_CASTLE_Q);
            }
        }
    }
    /* sliding pieces */
    from_bb = pos->bb[us][BISHOP] | pos->bb[us][QUEEN];
    while (from_bb) {
        from = bb_next(&from_bb);
        to_bb = hyperbola_bishop_moves(occ, from) & dest_squares;
        moves = moves_gen(moves, from, to_bb);
        //while(to_bb) {
        //    to = bb_next(&to_bb);
        //    *moves++ = move_make(from, to);
        //}
    }
    from_bb = pos->bb[us][ROOK] | pos->bb[us][QUEEN];
    while (from_bb) {
        from = bb_next(&from_bb);
        to_bb = hyperbola_rook_moves(occ, from) & dest_squares;
        moves = moves_gen(moves, from, to_bb);
        //while(to_bb) {
        //    to = bb_next(&to_bb);
        //    *moves++ = move_make(from, to);
        //}
    }

    /* knight */
    from_bb = pos->bb[us][KNIGHT];
    while (from_bb) {
        from = bb_next(&from_bb);
        to_bb = bb_knight_moves(dest_squares, from);
        moves = moves_gen(moves, from, to_bb);
        //while(to_bb) {
        //    to = bb_next(&to_bb);
        //    *moves++ = move_make(from, to);
        //}
    }

    /* pawn: relative rank and files */
    bitboard_t rel_rank8 = bb_rel_rank(RANK_8, us);
    bitboard_t rel_rank3 = bb_rel_rank(RANK_3, us);

    /* pawn: push */
    int shift = sq_up(us);
    tmp_bb = bb_shift(pos->bb[us][PAWN], shift) & empty;

    to_bb = tmp_bb & ~rel_rank8 & dest_squares;   /* non promotion */
    while(to_bb) {
        to = bb_next(&to_bb);
        from = to - shift;
        *moves++ = move_make(from, to);
    }
    to_bb = tmp_bb & rel_rank8 & dest_squares;    /* promotions */
    while(to_bb) {
        to = bb_next(&to_bb);
        from = to - shift;
        moves = move_gen_promotions(moves, from, to);
    }

    /* possible second push */
    to_bb = bb_shift(tmp_bb & rel_rank3, shift) & empty & dest_squares;
    while(to_bb) {
        to = bb_next(&to_bb);
        from = to - shift - shift;
        *moves++ = move_make_flags(from, to, M_DPUSH);
    }

    /* pawn: captures */
    /*
     * tmp_bb = pawn_attacks_bb(pos->bb[us][PAWN], us) & enemy_pieces;
     * //bb_print("FAIL", tmp_bb);
     * to_bb = tmp_bb & ~rel_rank8;
     * while (to_bb) {
     *     to = bb_next(&to_bb);
     *     from_bb = bb_pawn_attacks[them][to] & pos->bb[us][PAWN];
     *     while (from_bb) {
     *         from = bb_next(&from_bb);
     *         *moves++ = move_make(from, to);
     *     }
     * }
     * to_bb = tmp_bb & rel_rank8;
     * while (to_bb) {
     *     to = bb_next(&to_bb);
     *     from_bb = bb_pawn_attacks[them][to] & pos->bb[us][PAWN];
     *     while (from_bb) {
     *         from = bb_next(&from_bb);
     *         moves = move_make_promotions(moves, from, to);
     *     }
     * }
     */

    /* pawn: captures left */
    bitboard_t filter = ~bb_rel_file(FILE_A, us);
    shift = sq_upleft(us);
    tmp_bb = bb_shift(pos->bb[us][PAWN] & filter, shift) & enemy_pieces;

    to_bb = tmp_bb & ~rel_rank8;
    while (to_bb) {
        to = bb_next(&to_bb);
        from = to - shift;
        *moves++ = move_make(from, to);
    }
    to_bb = tmp_bb & rel_rank8;
    while (to_bb) {
        to = bb_next(&to_bb);
        from = to - shift;
        moves = move_gen_promotions(moves, from, to);
    }

    /* pawn: captures right */
    filter = ~bb_rel_file(FILE_H, us);
    shift = sq_upright(us);
    tmp_bb = bb_shift(pos->bb[us][PAWN] & filter, shift) & enemy_pieces;
    to_bb = tmp_bb & ~rel_rank8;
    while (to_bb) {
        to = bb_next(&to_bb);
        from = to - shift;
        *moves++ = move_make(from, to);
    }
    to_bb = tmp_bb & rel_rank8;
    while (to_bb) {
        to = bb_next(&to_bb);
        from = to - shift;
        moves = move_gen_promotions(moves, from, to);
    }

    /* pawn: en-passant
     * TODO: special case when in-check here ?
     */
    if ((to = pos->en_passant) != SQUARE_NONE) {
        from_bb = bb_pawn_attacks[them][to] & pos->bb[us][PAWN];
        while (from_bb) {
            from = bb_next(&from_bb);
            *moves++ = move_make_enpassant(from, to);
        }
    }

    /* TODO: add function per piece, and type, for easier debug
     */
finish:
    movelist->nmoves = moves - movelist->move;
    return movelist;
    //return movelist->nmoves = moves - movelist->move;
}
