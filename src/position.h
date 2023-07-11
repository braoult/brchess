/* position.h - position management definitions.
 *
 * Copyright (C) 2021 Bruno Raoult ("br")
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
#include <pool.h>
#include <list.h>
#include <bits.h>

#include "board.h"
#include "chessdefs.h"

typedef struct pos_s {
    u64 node_count;                               /* evaluated nodes */
    piece_t turn;                                 /* we use only color bit */
    castle_t castle;
    u16 clock_50;
    u16 curmove;
    eval_t eval;
    int eval_simple_phase;
    eval_t eval_simple;
    move_t *bestmove;
    bool moves_generated;
    bool moves_counted;
    board_t board[BOARDSIZE];

    square_t en_passant;

    bitboard_t bb[2][BB_END];                     /* use: pieces[BLACK][BB_PAWN] */
    bitboard_t occupied[2];                       /* OR of bb[COLOR][x] */
    bitboard_t controlled[2];
    u16 mobility[2];
    struct list_head pieces[2];                   /* pieces list, King is first */
    struct list_head moves[2];
} pos_t;

void bitboard_print(bitboard_t bb);
void bitboard_print2(bitboard_t bb1, bitboard_t bb2);
void pos_pieces_print(pos_t *pos);
void pos_bitboards_print(pos_t *pos);
void pos_print(pos_t *pos);
pos_t *pos_clear(pos_t *pos);
void pos_del(pos_t *pos);
pos_t *pos_startpos(pos_t *pos);
pos_t *pos_create();
pool_t *pos_pool_init();
void pos_pool_stats();
pos_t *pos_get();
pos_t *pos_dup(pos_t *pos);
void pos_check(pos_t *pos);

#endif  /* POSITION_H */
