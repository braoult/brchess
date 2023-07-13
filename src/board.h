/* board.h - board definitions.
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

#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>
#include "chessdefs.h"
#include "piece.h"

typedef struct board_s {
    piece_t piece;
    piece_list_t *s_piece;
    //struct list_head *s_piece;
} board_t;                                        /* 0x88 board */
#define BOARDSIZE    (8*8*2)

/* definitions for 0x88 representation
 */
#define SQ88(f, r)  (((r) << 4) | (f))            /* from rank,file to sq88 */
#define F88(s)      ((s) & 0x0f)                  /* from sq88 to file */
#define R88(s)      ((s) >> 4)                    /* from sq88 to rank */

#define SETF88(s, r) ((s) &= 0xf0, (s) |= (r))
#define SETR88(s, f) ((s) &= 0x0f, (s) |= (f)<<4)

#define SQ88_NOK(s)  ((s) & 0x88)                 /* invalid square */
#define SQ88_OK(s)   (!SQ88_NOK(s))

/* definitions for bitboard representation
 */
#define BB(f, r)     (1ULL << (8 * (r) + (f)))    /* from rank,file to bitboard */
#define SQ88_2_BB(s) (BB(F88(s), R88(s)))         /* from sq88 to bitboard */
#define FILEBB(b)    ((b) % 8)                    /* from sq88 to file */
#define RANKBB(b)    ((b) / 8)                    /* from sq88 to rank */

#define SQ88_NOK(s)  ((s) & 0x88)                 /* invalid square */
#define SQ88_OK(s)   (!SQ88_NOK(s))

/* piece human notation
 */
#define CHAR_EMPTY   ' '
#define CHAR_PAWN    'P'
#define CHAR_KNIGHT  'N'
#define CHAR_BISHOP  'B'
#define CHAR_ROOK    'R'
#define CHAR_QUEEN   'Q'
#define CHAR_KING    'K'

/* from human to machine
 */
#define C2FILE(c)    (tolower(c) - 'a')
#define C2RANK(c)    (tolower(c) - '1')
/* from machine to human
 */
#define FILE2C(f)    ((f) + 'a')
#define RANK2C(r)    ((r) + '1')

enum x88_square {
    x88_A1=0x00, x88_B1, x88_C1, x88_D1, x88_E1, x88_F1, x88_G1, x88_H1,
    x88_A2=0x10, x88_B2, x88_C2, x88_D2, x88_E2, x88_F2, x88_G2, x88_H2,
    x88_A3=0x20, x88_B3, x88_C3, x88_D3, x88_E3, x88_F3, x88_G3, x88_H3,
    x88_A4=0x30, x88_B4, x88_C4, x88_D4, x88_E4, x88_F4, x88_G4, x88_H4,
    x88_A5=0x40, x88_B5, x88_C5, x88_D5, x88_E5, x88_F5, x88_G5, x88_H5,
    x88_A6=0x50, x88_B6, x88_C6, x88_D6, x88_E6, x88_F6, x88_G6, x88_H6,
    x88_A7=0x60, x88_B7, x88_C7, x88_D7, x88_E7, x88_F7, x88_G7, x88_H7,
    x88_A8=0x70, x88_B8, x88_C8, x88_D8, x88_E8, x88_F8, x88_G8, x88_H8,
};

#endif  /* BOARD_H */
