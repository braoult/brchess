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

typedef u64 bitboard;
typedef ushort board;


/* flip a 0-63 square:
 * Vertical:   G8 (62) becomes G1 (6)
 * Horizontal: G8 (62) becomes B8 (57)
 */
#define FLIP_V(sq)      ((sq) ^ 56)
#define FLIP_H(sq)      ((sq) ^ 7)

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
