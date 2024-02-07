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

#include "brlib.h"

#include "chessdefs.h"
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
