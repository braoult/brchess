/* chessdefs.h - generic/catchall chess definitions.
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

#ifndef _CHESSDEFS_H
#define _CHESSDEFS_H

#include "brlib.h"                                /* brlib types */

#define ONE                1ull
#define C64(const_u64)     const_u64##ULL
#define mask(i)            ( (u64) (ONE << (i)) )

#define BOARDSIZE          (8*8)

/**
 * sq_rel - get relative square
 * @sq: white point of view square
 * @c:  color
 *
 * Get relative (mirrored if @c is BLACK) square.
 * Example: sq_rel(A1, WHITE) = A1, sq_rel(B2, BLACK) = B7
 *
 * @return: Relative square.
 */
#define sq_rel(sq, c)      ((square_t)((sq) ^ (56 * (c))))

/**
 * sq_rel_rank - get relative rank
 * @rank: white point of view rank
 * @c:  color
 *
 * Get relative (mirrored if @c is BLACK) rank.
 * Example: sq_rel(RANK_2, WHITE) = RANK_2, sq_rel(RANK_6, BLACK) = RANK_3
 *
 * @return: Relative rank.
 */
#define sq_rel_rank(rank, c)  ((rank_t)((7 * (c)) ^ rank))

/* castle_t bits structure
 */
typedef enum {
    CASTLE_WK = (1 << 0),                         /* 0x01 00000001 */
    CASTLE_WQ = (1 << 1),                         /* 0x02 00000010 */
    CASTLE_BK = (1 << 2),                         /* 0x04 00000100 */
    CASTLE_BQ = (1 << 3),                         /* 0x08 00001000 */

    CASTLE_W  = (CASTLE_WK | CASTLE_WQ),          /* 00000011 W castle mask */
    CASTLE_B  = (CASTLE_BK | CASTLE_BQ),          /* 00001100 B castle mask */

    CASTLE_K  = (1 << 0),                         /* generic K/Q, bits 0 and 1 */
    CASTLE_Q  = (1 << 1),
    CASTLE_KQ = (CASTLE_K |CASTLE_Q)
} castle_rights_t;

/* determine is oo or ooo is possible with castle flags f and color c
 */
//#define NORM_CASTLE(f, c)  ((f) >> (2 * (c)))      /* shift  flags to bits 0/1 */
//#define
//(NORM_CASTLE(f, c) & CASTLE_Q)
#define can_oo(f, c)     ((f) & (CASTLE_K  << ((c) * 2)))
#define can_ooo(f, c)    ((f) & (CASTLE_Q  << ((c) * 2)))
#define can_castle(f, c) ((f) & (CASTLE_KQ << ((c) * 2)))

#define clr_oo(f, c)     ((f) & ~(CASTLE_K  << (2 * (c))))
#define clr_ooo(f, c)    ((f) & ~(CASTLE_Q  << (2 * (c))))
#define clr_castle(f, c) ((f) & ~(CASTLE_KQ << (2 * (c)) ))

/* game phases
 */
#define OPENING    0
#define MIDDLEGAME 1
#define ENDGAME    2

/* forward defs */
typedef struct __pos_s pos_t;
typedef struct __movelist_s movelist_t;

/* basic types
 */
typedef u64 bitboard_t;

/* eval type
 */
//typedef s32 eval_t;

/* forward enum definition is impossible in C11, to simplify
 * cross-dependancies, all important enum are moved here.
 */
typedef enum {
    _SSQUARE_ = -1,                               /* force signed enum */
    A1 = 0, B1, C1, D1, E1, F1, G1, H1,
    A2,     B2, C2, D2, E2, F2, G2, H2,
    A3,     B3, C3, D3, E3, F3, G3, H3,
    A4,     B4, C4, D4, E4, F4, G4, H4,
    A5,     B5, C5, D5, E5, F5, G5, H5,
    A6,     B6, C6, D6, E6, F6, G6, H6,
    A7,     B7, C7, D7, E7, F7, G7, H7,
    A8,     B8, C8, D8, E8, F8, G8, H8,
    SQUARE_MAX = 64,
    SQUARE_NONE = 64
} square_t;

typedef enum {
    _SFILE_ = -1,                                 /* force signed enum */
    FILE_A = 0, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H,
    FILE_MAX,
} file_t;

typedef enum {
    _SRANK_ = -1,                                 /* force signed enum */
    RANK_1 = 0, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8,
    RANK_MAX,
} rank_t;

typedef enum {
    NORTH = 8,
    EAST =  1,
    SOUTH = -NORTH,
    WEST = -EAST,

    NORTH_EAST = (NORTH + EAST),
    SOUTH_EAST = (SOUTH + EAST),
    SOUTH_WEST = (SOUTH + WEST),
    NORTH_WEST = (NORTH + WEST),
} dir_t;


#endif  /* _CHESSDEFS_H */
