/* position.h - position management definitions.
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

#ifndef POSITION_H
#define POSITION_H

#include <stdint.h>

#include "brlib.h"
#include "bitops.h"

#include "bitboard.h"
#include "chessdefs.h"
#include "piece.h"

typedef struct {
    u64 node_count;                               /* evaluated nodes */
    int turn;                                     /* WHITE or BLACK */
    u16 clock_50;
    u16 plycount;                                 /* plies so far, start is 0 */
    square_t en_passant;
    castle_rights_t castle;

    //eval_t eval;
    //int check[2];
    //int eval_simple_phase;
    //eval_t eval_simple;
    //move_t *bestmove;
    //bool moves_generated;
    //bool moves_counted;

    bitboard_t bb[2][PIECE_TYPE_MAX];             /* bb[0][PAWN], bb[1][ALL_PIECES] */
    bitboard_t controlled[2];
    //u16 mobility[2];
                                                  //struct list_head pieces[2];                   /* pieces list, King is first */
    //struct list_head moves[2];
    piece_t board[BOARDSIZE];
} pos_t;

/**
 * pos_set_sq - unconditionally set a piece on a square
 * @pos: position
 * @square: square_t to set
 * @piece: piece_t to add
 *
 * Both position board and bitboards are modified.
 */
static inline void pos_set_sq(pos_t *pos, square_t square, piece_t piece)
{
    color_t color = COLOR(piece);
    piece_type_t type = PIECE(piece);
    pos->board[square] = piece;
    pos->bb[color][type] |= 1 << square;
    pos->bb[color][ALL_PIECES] |= 1 << square;
}

/**
 * pos_clr_sq - unconditionally remove a piece from square
 * @pos: position
 * @square: square_t to clear
 *
 * Both position board and bitboards are modified.
 */
static inline void pos_clr_sq(pos_t *pos, square_t square)
{
    piece_t piece = pos->board[square];
    piece_type_t type = PIECE(piece);
    color_t color = COLOR(piece);
    pos->board[square] = EMPTY;
    pos->bb[color][type] &= ~(1 << square);
    pos->bb[color][ALL_PIECES] &= ~(1 << square);
}

//void bitboard_print(bitboard_t bb, char *title);
//void bitboard_print2(bitboard_t bb1, bitboard_t bb2, char *title);

extern pos_t *pos_new();
extern pos_t *pos_dup(pos_t *pos);
extern void pos_del(pos_t *pos);
extern pos_t *pos_clear(pos_t *pos);

extern void pos_print(pos_t *pos);
extern void pos_pieces_print(pos_t *pos);

extern void raw_board_print(const pos_t *pos);

//extern pos_t *pos_startpos(pos_t *pos);

//void pos_check(position *pos);

#endif  /* POSITION_H */
