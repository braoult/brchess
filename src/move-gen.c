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

#include "board.h"
#include "bitboard.h"
#include "piece.h"
#include "position.h"
#include "move.h"
#include "hyperbola-quintessence.h"
#include "move-gen.h"


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

    bitboard_t my_pieces     = pos->bb[me][ALL_PIECES];
    bitboard_t not_my_pieces = ~my_pieces;
    bitboard_t enemy_pieces  = pos->bb[enemy][ALL_PIECES];

    bitboard_t occ           = my_pieces | enemy_pieces;
    bitboard_t empty         = ~occ;

    bitboard_t movebits, from_pawns;
    bitboard_t tmp1, tmp2;

    move_t *moves            = pos->moves.move;
    int nmoves               = pos->moves.nmoves;

    int from, to;

    /* sliding pieces */
    bit_for_each64(from, tmp1, pos->bb[me][BISHOP]) {
        movebits = hyperbola_bishop_moves(occ, from) & not_my_pieces;
        bit_for_each64(to, tmp2, movebits) {
            moves[nmoves++] = move_make(from, to);
        }
    }
    bit_for_each64(from, tmp1, pos->bb[me][ROOK]) {
        // printf("rook=%d/%s\n", from, sq_to_string(from));
        movebits = hyperbola_rook_moves(occ, from) & not_my_pieces;
        bit_for_each64(to, tmp2, movebits) {
            moves[nmoves++] = move_make(from, to);
        }
    }
    /* TODO: remove this one */
    bit_for_each64(from, tmp1, pos->bb[me][QUEEN]) {
        movebits = hyperbola_queen_moves(occ, from) & not_my_pieces;
        //bitboard_print("bishop", movebits);
        //movebits = hyperbola_rook_moves(occ, from);
        //bitboard_print("rook", movebits);
        //bitboard_print("diag", bb_diag[] movebits);
        // & not_my_pieces;
        bit_for_each64(to, tmp2, movebits) {
            moves[nmoves++] = move_make(from, to);
        }
    }

    /* knight */
    //bitboard_print("not_my_pieces", not_my_pieces);
    bit_for_each64(from, tmp1, pos->bb[me][KNIGHT]) {
        //printf("Nfrom=%d=%s\n", from, sq_to_string(from));
        movebits = bb_knight_moves(not_my_pieces, from);
        //printf("%lx\n", movebits);
        //bitboard_print("knight_moves", movebits);

        bit_for_each64(to, tmp2, movebits) {
            //printf("Nto=%d=%s\n", to, sq_to_string(to));
            moves[nmoves++] = move_make(from, to);
        }
    }

    /* king */
    from = pos->king[me];
    movebits = bb_king_moves(not_my_pieces, from);
    bit_for_each64(to, tmp2, movebits) {
        moves[nmoves++] = move_make(from, to);
    }

    /* pawn: relative rank and files */
    bitboard_t rel_rank7 = me == WHITE ? RANK_7bb : RANK_2bb;
    bitboard_t rel_rank3 = me == WHITE ? RANK_3bb : RANK_6bb;
    //bitboard_t rel_filea = (me == WHITE ? FILE_Abb : FILE_Hbb);
    //bitboard_t rel_fileh = (me == WHITE ? FILE_Hbb : FILE_Abb);
    int en_passant = pos->en_passant == SQUARE_NONE? 0: pos->en_passant;
    bitboard_t enemy_avail = bb_sq[en_passant] | enemy_pieces;

    /* pawn: ranks 2-6 push 1 and 2 squares */
    movebits = pawn_shift_up(pos->bb[me][PAWN] & ~rel_rank7, me) & empty;
    bit_for_each64(to, tmp1, movebits) {
        from = pawn_push_up(to, enemy);              /* reverse push */
        //printf("push %d->%d %s->%s", from, to, sq_to_string(from), sq_to_string(to));
        moves[nmoves++] = move_make(from, to);
    }
    movebits = pawn_shift_up(movebits & rel_rank3, me) & empty;
    bit_for_each64(to, tmp1, movebits) {
        from = pawn_push_up(pawn_push_up(to, enemy), enemy);
        moves[nmoves++] = move_make(from, to);
    }
    /* pawn: rank 7 push */
    movebits = pawn_shift_up(pos->bb[me][PAWN] & rel_rank7, me) & empty;
    bit_for_each64(to, tmp1, movebits) {
        from = pawn_push_up(to, enemy);              /* reverse push */
        moves[nmoves++] = move_make_promote(from, to, QUEEN);
        moves[nmoves++] = move_make_promote(from, to, ROOK);
        moves[nmoves++] = move_make_promote(from, to, BISHOP);
        moves[nmoves++] = move_make_promote(from, to, KNIGHT);
    }

    /* pawn: ranks 2-6 captures left, including en-passant */
    from_pawns = pos->bb[me][PAWN] & ~rel_rank7; // & ~rel_filea;
    movebits = pawn_shift_upleft(from_pawns, me) & enemy_avail;
    bit_for_each64(to, tmp1, movebits) {
        from = pawn_push_upleft(to, enemy);         /* reverse capture */
        moves[nmoves++] = move_make(from, to);
    }
    /* pawn: rank 7 captures left */
    from_pawns = pos->bb[me][PAWN] & rel_rank7; // & ~rel_filea;
    movebits = pawn_shift_upleft(from_pawns, me) & enemy_avail;
    bit_for_each64(to, tmp1, movebits) {
        from = pawn_push_upleft(to, enemy);         /* reverse capture */
        moves[nmoves++] = move_make_promote(from, to, QUEEN);
        moves[nmoves++] = move_make_promote(from, to, ROOK);
        moves[nmoves++] = move_make_promote(from, to, BISHOP);
        moves[nmoves++] = move_make_promote(from, to, KNIGHT);
    }

    /* pawn: ranks 2-6 captures right, including en-passant */
    from_pawns = pos->bb[me][PAWN] & ~rel_rank7; // & ~rel_fileh;
    movebits = pawn_shift_upright(from_pawns, me) & enemy_avail;
    bit_for_each64(to, tmp1, movebits) {
        from = pawn_push_upright(to, enemy);
        moves[nmoves++] = move_make(from, to);
    }
    /* pawn: rank 7 captures right */
    from_pawns = pos->bb[me][PAWN] & rel_rank7; // & ~rel_fileh;
    movebits = pawn_shift_upright(from_pawns, me) & enemy_pieces;
    bit_for_each64(to, tmp1, movebits) {
        from = pawn_push_upright(to, enemy);         /* reverse capture */
        moves[nmoves++] = move_make_promote(from, to, QUEEN);
        moves[nmoves++] = move_make_promote(from, to, ROOK);
        moves[nmoves++] = move_make_promote(from, to, BISHOP);
        moves[nmoves++] = move_make_promote(from, to, KNIGHT);
    }

    /* castle - Attention ! We consider that castle flags are correct,
     * only empty squares between K ans R are tested.
     */
    static struct {
        bitboard_t ooo;
        bitboard_t oo;
    } castle_empty[2] = {
        { B1bb | C1bb | D1bb, F1bb | G1bb },
        { B8bb | C8bb | D8bb, F8bb | G8bb }
    };
    static struct {
        square_t from;
        square_t ooo_to;
        square_t oo_to;
    } king_move[2] = {
        { .from=E1, .ooo_to=C1, .oo_to=G1 },
        { .from=E8, .ooo_to=C8, .oo_to=G8 },
    };
    /* so that bit0 is K castle flag, bit1 is Q castle flag */
    int castle_msk = pos->castle >> (2 * me);

    //int castle_msk = pos->castle >> (2 * color);
    if (castle_msk & CASTLE_WK && !(occ & castle_empty[me].oo)) {
        moves[nmoves++] = move_make(king_move[me].from, king_move[me].oo_to);
    }
    if (castle_msk & CASTLE_WQ && !(occ & castle_empty[me].ooo)) {
        moves[nmoves++] = move_make(king_move[me].from, king_move[me].ooo_to);
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
    return (pos->moves.nmoves = nmoves);
}
