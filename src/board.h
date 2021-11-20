/* board.h - board definitions.
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

/* necessary not to become mad to set bitboards
 */
enum bb_square{
    A1=(u64)1,     B1=(u64)A1<<1, C1=(u64)B1<<1, D1=(u64)C1<<1,
    E1=(u64)D1<<1, F1=(u64)E1<<1, G1=(u64)F1<<1, H1=(u64)G1<<1,

    A2=(u64)A1<<8, B2=(u64)B1<<8, C2=(u64)C1<<8, D2=(u64)D1<<8,
    E2=(u64)E1<<8, F2=(u64)F1<<8, G2=(u64)G1<<8, H2=(u64)H1<<8,

    A3=(u64)A2<<8, B3=(u64)B2<<8, C3=(u64)C2<<8, D3=(u64)D2<<8,
    E3=(u64)E2<<8, F3=(u64)F2<<8, G3=(u64)G2<<8, H3=(u64)H2<<8,

    A4=(u64)A3<<8, B4=(u64)B3<<8, C4=(u64)C3<<8, D4=(u64)D3<<8,
    E4=(u64)E3<<8, F4=(u64)F3<<8, G4=(u64)G3<<8, H4=(u64)H3<<8,

    A5=(u64)A4<<8, B5=(u64)B4<<8, C5=(u64)C4<<8, D5=(u64)D4<<8,
    E5=(u64)E4<<8, F5=(u64)F4<<8, G5=(u64)G4<<8, H5=(u64)H4<<8,

    A6=(u64)A5<<8, B6=(u64)B5<<8, C6=(u64)C5<<8, D6=(u64)D5<<8,
    E6=(u64)E5<<8, F6=(u64)F5<<8, G6=(u64)G5<<8, H6=(u64)H5<<8,

    A7=(u64)A6<<8, B7=(u64)B6<<8, C7=(u64)C6<<8, D7=(u64)D6<<8,
    E7=(u64)E6<<8, F7=(u64)F6<<8, G7=(u64)G6<<8, H7=(u64)H6<<8,

    A8=(u64)A7<<8, B8=(u64)B7<<8, C8=(u64)C7<<8, D8=(u64)D7<<8,
    E8=(u64)E7<<8, F8=(u64)F7<<8, G8=(u64)G7<<8, H8=(u64)H7<<8,
};

#endif  /* BOARD_H */
