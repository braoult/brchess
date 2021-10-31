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

typedef struct {
    piece_t piece;
    //piece_t *s_piece;
} board_t;                                        /* 0x88 board */
#define BOARDSIZE    (8*8*2)

/* definitions for 0x88 representation
 */
#define SQ88(f, r)   (16 * (r) + (f))             /* from rank,file to sq88 */
#define FILE88(s)    ((s) & 7)                    /* from sq88 to file */
#define RANK88(s)    ((s) >> 4)                   /* from sq88 to rank */

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

#endif
