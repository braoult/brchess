/* move-do.c - move do/undo.
 *
 * Copyright (C) 2021-2024 Bruno Raoult ("br")
 * Licensed under the GNU General Public License v3.0 or later.
 * Some rights reserved. See COPYING.
 *
 * You should have received a copy of the GNU General Public License along with this
 * program. If not, see <https://www.gnu.org/licenses/gpl-3.0-standalone.html>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later <https://spdx.org/licenses/GPL-3.0-or-later.html>
 *
 */

#include <malloc.h>
#include <ctype.h>
#include <stdlib.h>

#include "brlib.h"
#include "likely.h"

#include "chessdefs.h"
#include "move.h"
#include "position.h"

/**
 * move_do() - do move.
 * @pos:    &pos_t position
 * @pmove:  &move to apply
 * @state:  &state_t address where irreversible changes will be saved
 *
 * @pmove is applied to @pos:
 * - bitboards and board are updated
 * - counters are updated:
 *   - move count
 *   - 50-moves rule count
 * - flags are possibly updated:
 *   - castling
 *   - en-passant
 * @pmove is also modified in case of capture, to keep information about the
 * grabbed piece.
 *
 * @return: pos.
 */
pos_t *move_do(pos_t *pos, move_t *pmove, state_t *state)
{
#   ifdef DEBUG_MOVE
    //log(1, "new move: ");
    //move_print(0, move, M_PR_NL | M_PR_LONG);
#   endif
    move_t move = *pmove;
    *state = pos->state;                          /* save irreversible changes */

    color_t us = pos->turn, them = OPPONENT(us);
    square_t from = move_from(move), to = move_to(move);
    piece_t piece = pos->board[from];

    //piece_t piece = PIECE(move->piece), newpiece = piece, captured = move->capture;
    //int color = COLOR(move->piece);
    //u64 bb_from = SQ88_2_BB(from), bb_to = SQ88_2_BB(to);

    ++pos->clock_50;
    ++pos->plycount;

    if (is_capture(move)) {
        piece_t captured = pos->board[to];
        /* clear grabbed piece */
        if (!is_enpassant(move)) {
            pos_clr_sq(pos, to);
            *pmove = move_set_captured(move, PIECE(captured));
        } else {                                  /* special ep case */
            file_t ep_file = sq_file(pos->en_passant);
            square_t grabbed = sq_make(ep_file, sq_rel_rank(RANK_5, us));
            pos_clr_sq(pos, grabbed);
        }
        pos->clock_50 = 0;
    } else if (is_castle(move)) {                 /* handle rook move */
        square_t rookfrom, rookto;
        if (is_castle_K(move)) {
            rookfrom = sq_make(FILE_H, sq_rel_rank(RANK_1, us));
            rookto = to - 1;
        } else {
            rookfrom = sq_make(FILE_A, sq_rel_rank(RANK_1, us));
            rookto = to + 1;
        }
        pos_set_sq(pos, rookto, pos->board[rookfrom]);
        pos_clr_sq(pos, rookfrom);
        pos->castle = CLR_OO_OOO(pos->castle, us);
    } else if (is_dpush(move)) {                  /* if pawn double push, set e.p. */
        square_t ep = sq_make(sq_file(from), sq_rel_rank(RANK_3, us));
        pos->en_passant = ep;
    } else if (is_promotion(move)) {

    }

        CAN_CASTLE(pos->castle, us)) {     /* check for castling rights */
        if (from == sq_rel(E1, us)) {             /* king moved */
            pos->castle = CLR_OO_OOO(pos->castle, us);
        } else

    }
        switch (PIECE(piece)) {
            case PAWN:
                if (sq_rank(from) == sq_rel_rank(RANK_2, us) &&
                    sq_rank(to) == sq_rel_rank(RANK_4, us)) { /* 2 squares push */
                    square_t ep = sq_make(sq_file(from)) from + 8;
                }
            case KING:

        }
    }
    /* move piece itself */
    pos_set_sq(pos, to, piece);
    pos_clr_sq(pos, from);


            u64 bb_ep_grab = SQ88_2_BB(ep_grab);

            log_f(5, "en-passant=%d,%d\n", ep_file, color == WHITE ? 4 : 3);
            piece_del(&new->board[ep_grab].s_piece->list);
            new->board[ep_grab].piece = 0;
            new->occupied[OPPONENT(color)] &= ~bb_ep_grab;
            new->bb[OPPONENT(color)][BB_PAWN] &= ~bb_ep_grab;

        } else {
            piece_del(&new->board[to].s_piece->list);
            new->board[to].piece = 0;
            new->occupied[OPPONENT(color)] &= ~bb_to;
            new->bb[OPPONENT(color)][PIECETOBB(captured)] &= ~bb_to;
        }

    } else if (move->flags & M_CASTLE_Q) {
        uchar row = R88(from);
        square_t rook_from = SQ88(0, row);
        square_t rook_to = SQ88(3, row);
        u64 bb_rook_from = SQ88_2_BB(rook_from);
        u64 bb_rook_to = SQ88_2_BB(rook_to);

        new->board[rook_to] = new->board[rook_from];
        new->board[rook_to].s_piece->square = rook_to;
        new->occupied[color] &= ~bb_rook_from;
        new->occupied[color] |= bb_rook_to;
        new->bb[color][PIECETOBB(BB_ROOK)] &= ~bb_rook_from;
        new->bb[color][PIECETOBB(BB_ROOK)] |= bb_rook_to;
        new->board[rook_from].piece = 0;
        new->board[rook_from].s_piece = NULL;
        //new->castle &= color == WHITE? ~CASTLE_W: ~CASTLE_B;

    } else if (move->flags & M_CASTLE_K) {
        uchar row = R88(from);
        square_t rook_from = SQ88(7, row);
        square_t rook_to = SQ88(5, row);
        u64 bb_rook_from = SQ88_2_BB(rook_from);
        u64 bb_rook_to = SQ88_2_BB(rook_to);

        new->board[rook_to] = new->board[rook_from];
        new->board[rook_to].s_piece->square = rook_to;
        new->occupied[color] &= ~bb_rook_from;
        new->occupied[color] |= bb_rook_to;
        new->bb[color][PIECETOBB(BB_ROOK)] &= ~bb_rook_from;
        new->bb[color][PIECETOBB(BB_ROOK)] |= bb_rook_to;
        new->board[rook_from].piece = 0;
        new->board[rook_from].s_piece = NULL;
        // new->castle &= color == WHITE? ~CASTLE_W: ~CASTLE_B;
    }

    new->board[to] = new->board[from];
    /* fix dest square */
    new->board[to].s_piece->square = to;
    if (move->flags & M_PROMOTION) {
        log_f(5, "promotion to %s\n", P_SYM(move->promotion));
        log_f(5, "newpiece=%#x p=%#x\n", move->promotion, PIECE(move->promotion));
        newpiece = PIECE(move->promotion);
        new->board[to].piece = move->promotion;
        new->board[to].s_piece->piece = move->promotion;
    }
    /* replace old occupied bitboard by new one */
    new->occupied[color] &= ~bb_from;
    new->occupied[color] |= bb_to;
    new->bb[color][PIECETOBB(piece)] &= ~bb_from;
    new->bb[color][PIECETOBB(newpiece)] |= bb_to;
    if (move->flags & M_PROMOTION) {
        log_f(5, "promotion color=%d bbpiece=%d\n", color, PIECETOBB(newpiece));
        //bitboard_print(new->bb[color][PIECETOBB(newpiece)]);
    }
    /* set en_passant */
    new->en_passant = 0;
    if (piece == PAWN) {
        if (R88(from) == 1 && R88(to) == 3)
            pos->en_passant = SQ88(F88(from), 2);
        else if  (R88(from) == 6 && R88(to) == 4)
            pos->en_passant = SQ88(F88(from), 5);
    }

    /* always make "from" square empty */
    new->board[from].piece = 0;
    new->board[from].s_piece = NULL;

    //printf("old turn=%d ", color);
    //printf("new turn=%d\n", new->turn);
    //fflush(stdout);
    /* adjust castling flags */
    if ((bb_from | bb_to) & E1bb)
        new->castle &= ~(CASTLE_WQ | CASTLE_WK);
    else if ((bb_from | bb_to) & A1bb)
        new->castle &= ~CASTLE_WQ;
    else if ((bb_from | bb_to) & H1bb)
        new->castle &= ~CASTLE_WK;

    if ((bb_from | bb_to) & E8bb)
        new->castle &= ~(CASTLE_BQ | CASTLE_BK);
    else if ((bb_from | bb_to) & A8bb)
        new->castle &= ~CASTLE_BQ;
    else if ((bb_from | bb_to) & H8bb)
        new->castle &= ~CASTLE_BK;

    SET_COLOR(new->turn, OPPONENT(color));        /* pos color */
    return new;
}

void move_undo(pos_t *pos, __unused move_t move)
{
    pos_del(pos);
}
