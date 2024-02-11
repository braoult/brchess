/* bitboard.h - bitboard definitions.
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

#ifndef _BITBOARD_H
#define _BITBOARD_H

#include "brlib.h"
#include "bitops.h"

#include "chessdefs.h"
#include "board.h"

typedef u64 bitboard_t;

extern bitboard_t sq_bb[64], knight_attacks[64], king_attacks[64];

#define mask(i)        ( 1ULL << (i) )

typedef enum {
    FILE_Abb = 0x0101010101010101ULL,
    FILE_Bbb = 0x0202020202020202ULL,
    FILE_Cbb = 0x0404040404040404ULL,
    FILE_Dbb = 0x0808080808080808ULL,
    FILE_Ebb = 0x1010101010101010ULL,
    FILE_Fbb = 0x2020202020202020ULL,
    FILE_Gbb = 0x4040404040404040ULL,
    FILE_Hbb = 0x8080808080808080ULL,
} file_bb;

typedef enum {
    RANK_1bb = 0x00000000000000ffULL,
    RANK_2bb = 0x000000000000ff00ULL,
    RANK_3bb = 0x0000000000ff0000ULL,
    RANK_4bb = 0x00000000ff000000ULL,
    RANK_5bb = 0x000000ff00000000ULL,
    RANK_6bb = 0x0000ff0000000000ULL,
    RANK_7bb = 0x00ff000000000000ULL,
    RANK_8bb = 0xff00000000000000ULL
} rank_bb;

/* https://www.chessprogramming.org/Flipping_Mirroring_and_Rotating#Rotation
 */
static __always_inline bitboard_t bb_rotate_90(bitboard_t b)
{
    return b;
}

extern void bitboard_init(void);
extern void bitboard_print(const bitboard_t bitboard, const char *title);

#endif  /* _BITBOARD_H */
