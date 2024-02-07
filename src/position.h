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
//#include "pool.h"
//#include "list.h"
#include "bitops.h"
#include "bitboard.h"

#include "board.h"
#include "chessdefs.h"

typedef struct {
    u64 node_count;                               /* evaluated nodes */
    int turn;                                     /* WHITE or BLACK */
    u16 clock_50;
    u16 plycount;                                 /* plies so far, start is 0 */
    square en_passant;
    castle castle;

    //eval_t eval;
    //int check[2];
    //int eval_simple_phase;
    //eval_t eval_simple;
    //move_t *bestmove;
    //bool moves_generated;
    //bool moves_counted;

    bitboard bb[2][PIECE_TYPE_MAX];               /* bb[0][PAWN], bb[1][ALL_PIECES] */
    bitboard controlled[2];
    //u16 mobility[2];
    //struct list_head pieces[2];                   /* pieces list, King is first */
    //struct list_head moves[2];
    piece board[BOARDSIZE];
} position;

/**
 * pos_set_sq - unconditionally set a piece on a square
 * @pos: position
 * @p, @c: type and color of piece to add
 * @f, @r: destination file and rank
 *
 * Both position bords and bitboards are modified.
 */
static inline void pos_set_sq(position *pos, piece_type p, color c, file f, rank r)
{
    piece piece = MAKE_PIECE(p, c);
    square square = BB(f, r);
    pos->board[square] = piece;
    pos->bb[c][ALL_PIECES] |= 1 << square;
    pos->bb[c][p] |= 1 << square;
}

/**
 * pos_clr_sq - unconditionally remove a piece from square
 * @pos: position
 * @f, @r: destination file and rank
 *
 * Both position bords and bitboards are modified.
 */
static inline void pos_clr_sq(position *pos, file f, rank r)
{
    square square = BB(f, r);
    piece_type piece = PIECE(pos->board[square]);
    color color = COLOR(pos->board[square]);
    pos->board[square] = EMPTY;
    pos->bb[color][piece] &= ~(1 << square);
    pos->bb[color][ALL_PIECES] &= ~(1 << square);
}

//void bitboard_print(bitboard_t bb, char *title);
//void bitboard_print2(bitboard_t bb1, bitboard_t bb2, char *title);
void pos_pieces_print(position *pos);
//void pos_bitboards_print(pos_t *pos);
void pos_print(position *pos);
position *pos_clear(position *pos);
//void pos_del(position *pos);
position *pos_startpos(position *pos);
position *pos_new();
pool_t *pos_pool_init();
void pos_pool_stats();
position *pos_dup(position *pos);
//void pos_check(position *pos);

#endif  /* POSITION_H */
