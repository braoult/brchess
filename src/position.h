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

#include "chessdefs.h"
#include "bitboard.h"
#include "piece.h"
#include "move.h"
#include "board.h"

typedef struct __pos_s {
    u64 node_count;                               /* evaluated nodes */
    int turn;                                     /* WHITE or BLACK */
    u16 clock_50;
    u16 plycount;                                 /* plies so far, start is 0 */

    square_t king[2];                             /* dup with bb, faster retrieval */
    square_t en_passant;
    castle_rights_t castle;

    bitboard_t bb[2][PIECE_TYPE_MAX];             /* bb[0][PAWN], bb[1][ALL_PIECES] */
    bitboard_t controlled[2];                     /* unsure */
    bitboard_t checkers;                          /* opponent checkers */
    piece_t board[BOARDSIZE];
    movelist_t moves;
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
    pos->bb[color][type] |= mask(square);
    pos->bb[color][ALL_PIECES] |= mask(square);
    if (type == KING)
        pos->king[color] = square;
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
    pos->bb[color][type] &= ~mask(square);
    pos->bb[color][ALL_PIECES] &= ~mask(square);
}

//void bitboard_print(bitboard_t bb, char *title);
//void bitboard_print2(bitboard_t bb1, bitboard_t bb2, char *title);

extern pos_t *pos_new();
extern pos_t *pos_dup(const pos_t *pos);
extern void pos_del(pos_t *pos);
extern pos_t *pos_clear(pos_t *pos);

extern bitboard_t pos_checkers(const pos_t *pos, const color_t color);
extern char *pos_checkers2str(const pos_t *pos, char *str);

extern int pos_check(const pos_t *pos, const int strict);

extern void pos_print(const pos_t *pos);
extern void pos_print_mask(const pos_t *pos, const bitboard_t mask);
extern void pos_print_raw(const pos_t *pos, const int type);

extern void pos_print_pieces(const pos_t *pos);


#endif  /* POSITION_H */
