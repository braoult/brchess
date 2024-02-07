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

//static pool_t *pieces_pool;

const struct piece_details piece_details[] = {
    /*            W    B    SW   SB   Name     start value */
    [EMPTY]  = { ' ', ' ', " ", " ", "",
        0, 0, 0 },
    [PAWN]   = { 'P', 'p', "♙", "♟", "Pawn",
        PAWN_VALUE, PAWN_VALUE, PAWN_VALUE },
    [KNIGHT] = { 'N', 'n', "♘", "♞", "Knight",
        KNIGHT_VALUE, KNIGHT_VALUE, KNIGHT_VALUE },
    [BISHOP] = { 'B', 'b', "♗", "♝", "Bishop",
        BISHOP_VALUE, BISHOP_VALUE, BISHOP_VALUE },
    [ROOK]   = { 'R', 'r', "♖", "♜", "Rook",
        ROOK_VALUE, ROOK_VALUE, ROOK_VALUE },
    [QUEEN]  = { 'Q', 'q', "♕", "♛", "Queen",
        QUEEN_VALUE, QUEEN_VALUE, QUEEN_VALUE },
    [KING]   = { 'K', 'k', "♔", "♚", "King",
        KING_VALUE, KING_VALUE, KING_VALUE }
};

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
