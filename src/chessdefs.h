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

#ifndef _CHESSDEFS_H
#define _CHESSDEFS_H

#include "brlib.h"                                /* brlib types */

#define mask(i)        ( 1ULL << (i) )
#define C64(const_u64) const_u64##ULL
#define U64(const_s64) const_s64##LL

/* piece_t bits structure
 * piece is on bits 1-3, color on bit 4:
 * .... CPPP
 * C: 0 for white, 1: black
 * PPP: pawn (1), knight, bishop, rook, queen, king (6)
 */
typedef enum {
    WHITE, BLACK,
    COLOR_MAX
} color_t;

typedef enum {
    ALL_PIECES = 0,                               /* 'all pieces' bitboard */
    PAWN = 1, KNIGHT, BISHOP, ROOK, QUEEN, KING,
    PIECE_TYPE_MAX = 7                            /* bit 4 */
} piece_type_t;

typedef enum {
    EMPTY,
    W_PAWN = PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
    B_PAWN = PAWN | 8, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,
    PIECE_MAX
} piece_t;

#define OPPONENT(p)       !(p)

#define MASK_PIECE        0x07                      /* 00000111 */
#define MASK_COLOR        0x08                      /* 00001000 */

#define COLOR(p)          ((p) >> 3)              /* bitmask */
#define PIECE(p)          ((p) & MASK_PIECE)
#define MAKE_PIECE(p, c)  ((p) | (c) << 3)

#define IS_WHITE(p)       (!COLOR(p))
#define IS_BLACK(p)       (COLOR(p))

#define SET_WHITE(p)      ((p) &= ~MASK_COLOR)
#define SET_BLACK(p)      ((p) |= MASK_COLOR)
#define SET_COLOR(p, c)   (!(c)? SET_WHITE(p): SET_BLACK(p))

/* flip a 0-63 square:
 * Vertical:   G8 (62) becomes G1 (6)
 * Horizontal: G8 (62) becomes B8 (57)
 */
#define FLIP_V(sq)      ((sq) ^ 56)
#define FLIP_H(sq)      ((sq) ^ 7)

typedef u64 bitboard;
typedef ushort board;
#define BOARDSIZE    (8*8)
/* from human to machine */
#define C2FILE(c)    (tolower(c) - 'a')
#define C2RANK(c)    (tolower(c) - '1')
/* from machine to human */
#define FILE2C(f)    ((f) + 'a')
#define RANK2C(r)    ((r) + '1')

/* castle_t bits structure
 */
typedef enum {
    CASTLE_WK = (1 << 0),                         /* 0x01 00000001 */
    CASTLE_WQ = (1 << 1),                         /* 0x02 00000010 */
    CASTLE_BK = (1 << 2),                         /* 0x04 00000100 */
    CASTLE_BQ = (1 << 3),                         /* 0x08 00001000 */
} castle_rights_t;

#define CASTLE_W        (CASTLE_WK | CASTLE_WQ)   /* 00000011 W castle mask */
#define CASTLE_B        (CASTLE_BK | CASTLE_BQ)   /* 00001100 B castle mask */

/* game phases
 */
#define OPENING    0
#define MIDDLEGAME 1
#define ENDGAME    2

/* bitboard
 */
//typedef u64 bitboard_t;

/* eval type
 */
//typedef s32 eval_t;

/* forward typedefs
 */
//typedef struct piece_list_s piece_list_t;
//typedef struct board_s board_t;
//typedef struct pos_s pos_t;
//typedef struct move_s move_t;

#endif  /* _CHESSDEFS_H */
