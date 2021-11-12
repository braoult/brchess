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
    E_PAWN,
    E_KNIGHT,
    E_BISHOP,
    E_ROOK,
    E_QUEEN,
    E_KING,
    E_COLOR = 8
};


/* piece bitmask in piece_t
 */
enum {
    EMPTY  = 0,
    PAWN   = 1 << (E_PAWN - 1),                   /* 0x01 00000001 */
    KNIGHT = 1 << (E_KNIGHT - 1),                 /* 0x02 00000010 */
    BISHOP = 1 << (E_BISHOP - 1),                 /* 0x04 00000100 */
    ROOK   = 1 << (E_ROOK - 1),                   /* 0x08 00001000 */
    QUEEN  = 1 << (E_QUEEN - 1),                  /* 0x10 00010000 */
    KING   = 1 << (E_KING - 1),                   /* 0x20 00100000 */
};

#define WHITE           0                         /* 0x00 00000000 */
#define BLACK           1                         /* 0x01 00000001 */
#define OPPONENT(p)     !(p)

#define MASK_PIECE      0x3F                      /* 00111111 */
#define MASK_COLOR      0x80                      /* 10000000 */

#define COLOR(p)        ((p) & MASK_COLOR)        /* bitmask */
#define VCOLOR(p)       (!!COLOR(p))              /* WHITE/BLACK */
#define PIECE(p)        ((p) & MASK_PIECE)
#define E_PIECE(p)      (ffs64(PIECE(p)))         /* convert mask to E_XX */

#define IS_WHITE(p)     (!COLOR(p))
#define IS_BLACK(p)     (COLOR(p))

#define SET_WHITE(p)    ((p) &= ~MASK_COLOR)
#define SET_BLACK(p)    ((p) |= MASK_COLOR)
#define SET_COLOR(p, c) (!(c)? SET_WHITE(p): SET_BLACK(p))

/* square_t bits structure : ffffrrrr
 * ffff: file
 * rrrr: rank
 */
typedef unsigned char square_t;

#define GET_R(s)        ((s) >> 4)
#define GET_F(s)        ((s) & 0x0f)
#define SET_R(s, f)     ((s) &= 0x0f, (s) |= (f)<<4)
#define SET_F(s, r)     ((s) &= 0xf0, (s) |= (r))
#define SQUARE(f, r)    ((r) << 4 | (f))

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
