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

#include <stdint.h>

/* piece_t bits structure
 */
typedef unsigned char piece_t;

#define EMPTY           0

#define WHITE           0                         /* 0x00 00000000 */
#define BLACK           1                         /* 0x01 00000001 */

#define PAWN            (1 << 1)                  /* 0x02 00000010 */
#define KNIGHT          (1 << 2)                  /* 0x04 00000100 */
#define BISHOP          (1 << 3)                  /* 0x08 00001000 */
#define ROOK            (1 << 4)                  /* 0x10 00010000 */
#define QUEEN           (1 << 5)                  /* 0x20 00100000 */
#define KING            (1 << 6)                  /* 0x40 01000000 */

#define MASK_COLOR      0x01                      /* 00000001 */
#define MASK_PIECE      0x7E                      /* 01111110 */

#define COLOR(p)        ((p) & MASK_COLOR)
#define PIECE(p)        ((p) & MASK_PIECE)

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

#define GET_F(s)        ((s) >> 4)
#define GET_R(s)        ((s) & 0x0f)
#define SET_F(s, f)     ((s) &= 0x0f, (s) |= (f)<<4)
#define SET_R(s, r)     ((s) &= 0xf0, (s) |= (r))
#define SQUARE(f, r)    ((f) << 4 | (r))

/* castle_t bits structure
 */
typedef unsigned char castle_t;

#define CASTLE_WK       1                         /* 0x01 00000001 */
#define CASTLE_WQ       (1 << 1)                  /* 0x02 00000010 */
#define CASTLE_BK       (1 << 2)                  /* 0x04 00000100 */
#define CASTLE_BQ       (1 << 3)                  /* 0x08 00001000 */

#define CASTLE_W        0x03                      /* 00000011 W castle mask */
#define CASTLE_B        0x0C                      /* 00001100 B castle mask */

/* eval type
 */
typedef int64_t         eval_t;

#endif
