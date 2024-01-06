/* chessdefs.h - generic chess definitions.
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

#ifndef CHESSDEFS_H
#define CHESSDEFS_H

#include "br.h"

/* piece_t bits structure
 * MSB 8 7 6 5 4 3 2 1 LSB
 * 1: color (0 for white)
 * 2-7: bit set for pawn (2), knight, bishop, rook, queen, king (7)
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

/* pos_t bitboards tables
 */
enum {
    BB_ALL = 0,                                   /* OR of all bitboards */
    BB_UNUSED,                                    /* future use ? */
    BB_PAWN = E_PAWN,
    BB_KNIGHT,
    BB_BISHOP,
    BB_ROOK,
    BB_QUEEN,
    BB_KING,
    BB_END
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

#define PIECETOBB(p)    (ffs64(PIECE(p)))         /* from piece_t to bb piece array */

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

/* flip a 0-63 square:
 * Vertical:   G8 (62) becomes G1 (6)
 * Horizontal: G8 (62) becomes B8 (57)
 */
#define FLIP_V(sq)      ((sq) ^ 56)
#define FLIP_H(sq)      ((sq) ^ 7)
/* square_t bits structure : rrrrffff
 * ffff: file
 * rrrr: rank
 */
typedef unsigned char square_t;

/* castle_t bits structure
 */
typedef unsigned char castle_t;

#define CASTLE_WK       (1 << 0)                  /* 0x01 00000001 */
#define CASTLE_WQ       (1 << 1)                  /* 0x02 00000010 */
#define CASTLE_BK       (1 << 2)                  /* 0x04 00000100 */
#define CASTLE_BQ       (1 << 3)                  /* 0x08 00001000 */

#define CASTLE_W        (CASTLE_WK | CASTLE_WQ)   /* 00000011 W castle mask */
#define CASTLE_B        (CASTLE_BK | CASTLE_BQ)   /* 00001100 B castle mask */

/* game phases
 */
#define OPENING    0
#define MIDDLEGAME 1
#define ENDGAME    2

/* bitboard
 */
typedef u64 bitboard_t;


/* eval type
 */
typedef s32 eval_t;

/* forward typedefs
 */
typedef struct piece_list_s piece_list_t;
typedef struct board_s board_t;
typedef struct pos_s pos_t;
typedef struct move_s move_t;

#endif
