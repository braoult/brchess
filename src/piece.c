/* piece.c - piece list management.
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
#include <string.h>
#include <assert.h>

//#include <debug.h>
//#include <pool.h>
//#include <list.h>

#include "chessdefs.h"
#include "piece.h"
//#include "board.h"
//#include "bitboard.h"
//#include "position.h"

/**
 * piece_details
 */
const struct piece_details piece_details[] = {
    /*             Abb  Sym  Name   start value */
    [EMPTY]    = { ' ', " ", " ", 0, 0, 0 },
    [W_PAWN]   = { 'P', "♙", "Pawn",   P_VAL_OPN, P_VAL_MID, P_VAL_END },
    [W_KNIGHT] = { 'N', "♘", "Knight", N_VAL_OPN, N_VAL_MID, N_VAL_END },
    [W_BISHOP] = { 'B', "♗", "Bishop", B_VAL_OPN, B_VAL_MID, B_VAL_END },
    [W_ROOK]   = { 'R', "♖", "Rook",   R_VAL_OPN, R_VAL_MID, R_VAL_END },
    [W_QUEEN]  = { 'Q', "♕", "Queen",  Q_VAL_OPN, Q_VAL_MID, Q_VAL_END },
    [W_KING]   = { 'K', "♔", "King",   K_VAL_OPN, K_VAL_MID, K_VAL_END },
    [7]        = { '7', "�", "Inv 7",  0, 0, 0 },
    [8]        = { '7', "�", "Inv 8",  0, 0, 0 },
    [B_PAWN]   = { 'p', "♟", "Pawn",   P_VAL_OPN, P_VAL_MID, P_VAL_END },
    [B_KNIGHT] = { 'n', "♞", "Knight", P_VAL_OPN, N_VAL_MID, N_VAL_END },
    [B_BISHOP] = { 'b', "♝", "Bishop", P_VAL_OPN, B_VAL_MID, B_VAL_END },
    [B_ROOK]   = { 'r', "♜", "Rook",   P_VAL_OPN, R_VAL_MID, R_VAL_END },
    [B_QUEEN]  = { 'q', "♛", "Queen",  P_VAL_OPN, Q_VAL_MID, Q_VAL_END },
    [B_KING]   = { 'k', "♚", "King",   P_VAL_OPN, K_VAL_MID, K_VAL_END },
};

const char *fenpieces_idx = " PNBRQ  pnbrq";
/*
 * void piece_list_print(struct list_head *list)
 * {
 *     struct list_head *p_cur, *tmp;
 *     piece_list_t *piece;
 *
 *     list_for_each_safe(p_cur, tmp, list) {
 *         piece = list_entry(p_cur, piece_list_t, list);
 *
 *         printf("%s%c%c ", P_SYM(piece->piece),
 *                FILE2C(F88(piece->square)),
 *                RANK2C(R88(piece->square)));
 *     }
 *     printf("\n");
 * }
 *
 * pool_t *piece_pool_init()
 * {
 *     if (!pieces_pool)
 *         pieces_pool = pool_create("pieces", 128, sizeof(piece_list_t));
 *     return pieces_pool;
 * }
 *
 * void piece_pool_stats()
 * {
 *     if (pieces_pool)
 *         pool_stats(pieces_pool);
 * }
 *
 * piece_list_t *piece_add(pos_t *pos, piece_t piece, square_t square)
 * {
 *     piece_list_t *new;
 *     short color = COLOR(piece);
 *
 * #   ifdef DEBUG_PIECE
 *     log_f(3, "piece=%02x square=%02x\n", piece, square);
 *     log_f(5, "Adding %s %s on %c%c\n", color? "Black": "White",
 *           P_NAME(piece), FILE2C(F88(square)), RANK2C(R88(square)));
 * #   endif
 *     if ((new = pool_get(pieces_pool))) {
 *         /\* first piece is always king *\/
 *         if (PIECE(piece) == KING)
 *             list_add(&new->list, &pos->pieces[color]);
 *         else
 *             list_add_tail(&new->list, &pos->pieces[color]);
 *         new->piece = piece;
 *         new->square = square;
 *         new->castle = 0;
 *         new-> value = piece_details[PIECE(piece)].value;
 *     }
 *
 *     return new;
 * }
 *
 * void piece_del(struct list_head *ptr)
 * {
 *     piece_list_t *piece = list_entry(ptr, piece_list_t, list);
 * #   ifdef DEBUG_PIECE
 *     log_f(3, "piece=%02x square=%02x\n", piece->piece, piece->square);
 * #   endif
 *     list_del(ptr);
 *     pool_add(pieces_pool, piece);
 *     return;
 * }
 *
 * int pieces_del(pos_t *pos, short color)
 * {
 *     struct list_head *p_cur, *tmp, *head;
 *     int count = 0;
 *
 *     head = &pos->pieces[color];
 *
 *     list_for_each_safe(p_cur, tmp, head) {
 *         piece_del(p_cur);
 *         count++;
 *     }
 * #   ifdef DEBUG_PIECE
 *     log_f(3, "color=%d removed=%d\n", color, count);
 * #   endif
 *     return count;
 * }
 */
