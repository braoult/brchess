/* chessdefs.h - generic chess definitions.
 *
 * Copyright (C) 2021 Bruno Raoult ("br")
 * Licensed under the GNU General Public License v3.0 or later.
 * Some rights reserved. See COPYING.
 *
 * You should have received a copy of the GNU General Public License along with this
 * program. If not, see <https://www.gnu.org/licenses/gpl-3.0-standalone.htmlL>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later <https://spdx.org/licenses/GPL-3.0-or-later.html>
 *
 */

#ifndef CHESSDEFS_H
#define CHESSDEFS_H

#include "bits.h"

/* piece_t bits structure
 */
typedef u8 piece_t;

enum {
    E_EMPTY = 0,
    E_COLOR,                                      /* LSB */
    E_PAWN,
    E_KNIGHT,
    E_BISHOP,
    E_ROOK,
    E_QUEEN,
    E_KING,
};


/* piece bitmask in piece_t
 */
enum {
    EMPTY  = 0,
    PAWN   = 1 << (E_PAWN - 1),                   /* 1<<(2-1) = 0x02 00000010 */
    KNIGHT = 1 << (E_KNIGHT - 1),                 /* 0x04 00000100 */
    BISHOP = 1 << (E_BISHOP - 1),                 /* 0x08 00001000 */
    ROOK   = 1 << (E_ROOK - 1),                   /* 0x10 00010000 */
    QUEEN  = 1 << (E_QUEEN - 1),                  /* 0x20 00100000 */
    KING   = 1 << (E_KING - 1),                   /* 0x40 01000000 */
};

#define WHITE           0                         /* 0x00 00000000 */
#define BLACK           1                         /* 0x01 00000001 */
#define OPPONENT(p)     !(p)

#define MASK_COLOR      0x01                      /* 00000001 */
#define MASK_PIECE      0x7E                      /* 01111110 */

#define COLOR(p)        ((p) & MASK_COLOR)        /* bitmask */
#define PIECE(p)        ((p) & MASK_PIECE)
#define E_PIECE(p)      (ffs64(PIECE(p)))         /* convert mask to E_XX */

#define IS_WHITE(p)     (!COLOR(p))
#define IS_BLACK(p)     (COLOR(p))

#define SET_WHITE(p)    ((p) &= ~MASK_COLOR)
#define SET_BLACK(p)    ((p) |= MASK_COLOR)
#define SET_COLOR(p, c) (!(c)? SET_WHITE(p): SET_BLACK(p))

/* square_t bits structure : rrrrffff
 * ffff: file
 * rrrr: rank
 */
typedef unsigned char square_t;

/* castle_t bits structure
 */
typedef unsigned char castle_t;

#define CASTLE_WK       1                         /* 0x01 00000001 */
#define CASTLE_WQ       (1 << 1)                  /* 0x02 00000010 */
#define CASTLE_BK       (1 << 2)                  /* 0x04 00000100 */
#define CASTLE_BQ       (1 << 3)                  /* 0x08 00001000 */

#define CASTLE_W        0x03                      /* 00000011 W castle mask */
#define CASTLE_B        0x0C                      /* 00001100 B castle mask */

/* bitboard
 */
typedef uint64_t bitboard_t;


/* eval type
 */
typedef int64_t eval_t;

/* forward typedefs
 */
typedef struct piece_list_s piece_list_t;
typedef struct board_s board_t;
typedef struct pos_s pos_t;

#endif
