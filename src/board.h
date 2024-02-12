/* board.h - 8x8 board definitions.
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

#ifndef BOARD_H
#define BOARD_H

#include "brlib.h"

#include "chessdefs.h"

typedef enum {
    _SSQUARE_ = -1,                               /* force signed enum */
    A1 = 0, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
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

/* flip a 0-63 square:
 * Vertical:   G8 (62) becomes G1 (6)
 * Horizontal: G8 (62) becomes B8 (57)
 */
#define FLIP_V(sq)      ((sq) ^ 56)
#define FLIP_H(sq)      ((sq) ^ 7)
#define FLIP_HV(sq)     (FLIP_V(FLIP_H(sq)))

/* TODO: revert to macros after bitboard migration */
static __always_inline square_t sq_make(file_t file, rank_t rank)
{
    return (rank << 3) + file;
}
static __always_inline file_t sq_file(square_t square)
{
    return square & 7;
}
static __always_inline rank_t sq_rank(square_t square)
{
    return square >> 3;
}


#define sq_ok(sq)       ((sq) >= A1 && (sq) <= H8)
#define sq_coord_ok(c)  ((c) >= 0 && (c) < 8)

/* Chebyshev distance: https://www.chessprogramming.org/Distance */
#define sq_dist(sq1, sq2) (max(abs(sq_file(sq2) - sq_file(sq1)),  \
                               abs(sq_rank(sq2) - sq_rank(sq1))))
/* Manhattan distance */
#define sq_manh(sq1, sq2) (abs(sq_file(sq2) - sq_file(sq1)) +     \
                           abs(sq_rank(sq2) - sq_rank(sq1)))

// while the orthogonal Manhattan-Distance is the sum of both absolute rank- and file-distance distances.

//Dtaxi = |r2 - r1| + |f2 - f1|

// Chebyshev Distance

//#include "piece.h"

/* definitions for 0x88 representation
 */
//#define SQ88(f, r)  (((r) << 4) | (f))            /* from rank,file to sq88 */
//#define F88(s)      ((s) & 0x0f)                  /* from sq88 to file */
//#define R88(s)      ((s) >> 4)                    /* from sq88 to rank */

//#define SETF88(s, r) ((s) &= 0xf0, (s) |= (r))
//#define SETR88(s, f) ((s) &= 0x0f, (s) |= (f)<<4)

//#define SQ88_NOK(s)  ((s) & 0x88)                 /* invalid square */
//#define SQ88_OK(s)   (!SQ88_NOK(s))

/* definitions for bitboard representation
 */
//#define BB(f, r)     (1ULL << (8 * (r) + (f)))    /* from rank,file to bitboard */


//#define SQ88_2_BB(s) (BB(F88(s), R88(s)))         /* from sq88 to bitboard */
//#define FILEBB(b)    ((b) % 8)                    /* from sq88 to file */
//#define RANKBB(b)    ((b) / 8)                    /* from sq88 to rank */

//#define SQ88_NOK(s)  ((s) & 0x88)                 /* invalid square */
//#define SQ88_OK(s)   (!SQ88_NOK(s))

/* from human to machine
 */
/*
 * #define C2FILE(c)    (tolower(c) - 'a')
 * #define C2RANK(c)    (tolower(c) - '1')
 * /\* from machine to human
 *  *\/
 * #define FILE2C(f)    ((f) + 'a')
 * #define RANK2C(r)    ((r) + '1')
 */

#endif  /* BOARD_H */
