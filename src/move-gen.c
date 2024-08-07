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
#include "hq.h"
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
    color_t  us       = pos->turn;
    color_t them      = OPPONENT(us);
    square_t from     = move_from(move);
    square_t to       = move_to(move);
    square_t kingsq   = pos->king[us];
    square_t ep       = pos->en_passant;
    bitboard_t kingbb = pos->bb[us][KING];
    bitboard_t occ    = pos_occ(pos);
    u64 pinned        = BIT(from) & pos->blockers;
    u64 checkers      = pos->checkers;

    bug_on(pos->board[from] == NO_PIECE || COLOR(pos->board[from]) != us);

    /* (1) - Castling & King
     * For castling, we need to check intermediate squares attacks only.
     * Attention: To test if K is in check after moving, we need to exclude
     * king from occupation bitboard (to catch king moving away from checker
     * on same line) !
     */
    if (is_castle(move)) {
        int dir = to > from? 1: -1;
        if (sq_is_attacked(pos, occ, from + dir, them))
            return false;
    }
    if (from == kingsq) {
        return !sq_is_attacked(pos, occ ^ kingbb, to, them);
    }

    /* (2) - King is in check
     * Double-check is already handled in (1), as only K moves were generated
     * by pseudo legal move generator.
     * Special cases (illegal):
     *   - e.p., if the grabbed pawn is *not* giving check
     *   - piece is pinned
     */
    if (checkers) {
        if (pinned)
            return false;
        if (is_enpassant(move)) {
            return ep + sq_up(them) == ctz64(checkers);
        }
        return true;
    }

    /* (3) - pinned pieces
     * We verify here that pinned piece P stays on line between K & dest square.
     */
    if (pinned) {
        return bb_line[from][kingsq] & BIT(to);   /* is to on pinner line ? */
    }

    /* (4) - En-passant
     * pinned piece is already handled in (3).
     * One case not handled anywhere else: when the two "disappearing" pawns
     * would discover a R/Q horizontal check.
     * Note: grabbed pawn *cannot* discover a check (impossible position).
     */
    if (is_enpassant(move)) {
        bitboard_t rank5 = bb_rel_rank(RANK_5, us);

        if (kingbb & rank5) {
            bitboard_t exclude = BIT(ep + sq_up(them)) | BIT(from);
            bitboard_t rooks = (pos->bb[them][ROOK] | pos->bb[them][QUEEN]) & rank5;

            return !(hq_rank_moves(occ ^ exclude, kingsq) & rooks);
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
 *  - castling: M_CASTLE
 *  - en-passant: M_EN_PASSANT
 *  - promotion: M_PROMOTION
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
static inline __unused move_t *moves_gen_flags(move_t *moves, square_t from,
                                               bitboard_t to_bb,
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
 * Actual promoted piece type is encoded as piece - 2, i.e. N = 0, B = 1,
 * R = 2, Q = 3.
 *
 * @Return: New @moves.
 */
static inline move_t *move_gen_promotions(move_t *moves, square_t from, square_t to)
{
    /* your attention: "downto operator" */
    for (piece_type_t pt = QUEEN - 1; pt --> KNIGHT - 2;)
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
    move_t *moves            = movelist->move;
    square_t king = pos->king[us];

    bitboard_t from_bb, to_bb;
    bitboard_t tmp_bb;
    square_t from, to;

    /* king - MUST BE FIRST */
    to_bb = bb_king_moves(dest_squares, king);
    moves = moves_gen(moves, king, to_bb);

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
        /* For castle, we check the opponent attacks on squares between from and to.
         * To square attack check will be done in gen_is_legal.
         */
        if (can_oo(pos->castle, us)) {

            /* CHANGE HERE, either with bitmask >> or direct sq check */
            bitboard_t occmask = rel_rank1 & (FILE_Fbb | FILE_Gbb);
            if (!(occ & occmask)) {
                *moves++ = move_make_flags(king, king + 2, M_CASTLE);
            }
        }
        if (can_ooo(pos->castle, us)) {
            bitboard_t occmask = rel_rank1 & (FILE_Bbb | FILE_Cbb | FILE_Dbb);
            if (!(occ & occmask)) { // &&
                *moves++ = move_make_flags(king, king - 2, M_CASTLE);
            }
        }
    }

    /* sliding pieces */
    from_bb = pos->bb[us][BISHOP] | pos->bb[us][QUEEN];
    while (from_bb) {
        from = bb_next(&from_bb);
        to_bb = hq_bishop_moves(occ, from) & dest_squares;
        moves = moves_gen(moves, from, to_bb);
    }
    from_bb = pos->bb[us][ROOK] | pos->bb[us][QUEEN];
    while (from_bb) {
        from = bb_next(&from_bb);
        to_bb = hq_rook_moves(occ, from) & dest_squares;
        moves = moves_gen(moves, from, to_bb);
    }

    /* knight */
    from_bb = pos->bb[us][KNIGHT];
    while (from_bb) {
        from = bb_next(&from_bb);
        to_bb = bb_knight_moves(dest_squares, from);
        moves = moves_gen(moves, from, to_bb);
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
        *moves++ = move_make(from, to);
    }

    /* pawn: captures */
    tmp_bb = bb_pawns_attacks(pos->bb[us][PAWN], shift) & enemy_pieces;
    to_bb = tmp_bb & ~rel_rank8;
    while (to_bb) {
        to = bb_next(&to_bb);
        from_bb = bb_pawn_attacks[them][to] & pos->bb[us][PAWN];
        while (from_bb) {
            from = bb_next(&from_bb);
            *moves++ = move_make(from, to);
        }
    }
    to_bb = tmp_bb & rel_rank8;
    while (to_bb) {
        to = bb_next(&to_bb);
        from_bb = bb_pawn_attacks[them][to] & pos->bb[us][PAWN];
        while (from_bb) {
            from = bb_next(&from_bb);
            moves = move_gen_promotions(moves, from, to);
        }
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

/**
 * pos_gen_legal() - generate position legal moves
 * @pos: position
 * @movelist: &movelist_t array to store moves
 *
 * Generate all @pos legal moves for player-to-move.
 * @movelist is filled with the moves.
 *
 * @Return: movelist
 */
movelist_t *pos_gen_legal(pos_t *pos, movelist_t *movelist)
{
    return pos_legal(pos, pos_gen_pseudo(pos, movelist));
}
