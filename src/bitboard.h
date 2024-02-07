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
#include "chessdefs.h"
#include "piece.h"
#include "bitops.h"

typedef enum {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
    SQUARE_MAX = 64,
    SQUARE_NONE = 64
} square;

typedef enum {
    FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H,
    FILE_MAX,
} file;

typedef enum {
    RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8,
    RANK_MAX,
} rank;

typedef enum {
    //A1 = 0x01ULL, B1 = 0x02ULL, C1 = 1UL <<  2, D1 = 1UL <<  3,
    //E1 = 1UL <<  4, F1 = 1UL <<  5, G1 = 1UL <<  6, H1 = 1UL <<
    A1bb = mask(A1), A2bb = mask(A2), A3bb = mask(A3), A4bb = mask(A4),
    A5bb = mask(A5), A6bb = mask(A6), A7bb = mask(A7), A8bb = mask(A8),
    B1bb = mask(B1), B2bb = mask(B2), B3bb = mask(B3), B4bb = mask(B4),
    B5bb = mask(B5), B6bb = mask(B6), B7bb = mask(B7), B8bb = mask(B8),
    C1bb = mask(C1), C2bb = mask(C2), C3bb = mask(C3), C4bb = mask(C4),
    C5bb = mask(C5), C6bb = mask(C6), C7bb = mask(C7), C8bb = mask(C8),
    D1bb = mask(D1), D2bb = mask(D2), D3bb = mask(D3), D4bb = mask(D4),
    D5bb = mask(D5), D6bb = mask(D6), D7bb = mask(D7), D8bb = mask(D8),
    E1bb = mask(E1), E2bb = mask(E2), E3bb = mask(E3), E4bb = mask(E4),
    E5bb = mask(E5), E6bb = mask(E6), E7bb = mask(E7), E8bb = mask(E8),
    F1bb = mask(F1), F2bb = mask(F2), F3bb = mask(F3), F4bb = mask(F4),
    F5bb = mask(F5), F6bb = mask(F6), F7bb = mask(F7), F8bb = mask(F8),
    G1bb = mask(G1), G2bb = mask(G2), G3bb = mask(G3), G4bb = mask(G4),
    G5bb = mask(G5), G6bb = mask(G6), G7bb = mask(G7), G8bb = mask(G8),
    H1bb = mask(H1), H2bb = mask(H2), H3bb = mask(H3), H4bb = mask(H4),
    H5bb = mask(H5), H6bb = mask(H6), H7bb = mask(H7), H8bb = mask(H8),
} sq_bb;

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

typedef enum {
    NORTH = 8,
    EAST =  1,
    SOUTH = -NORTH,
    WEST = -EAST,

    NORTH_EAST = (NORTH + EAST),
    SOUTH_EAST = (SOUTH + EAST),
    SOUTH_WEST = (SOUTH + WEST),
    NORTH_WEST = (NORTH + WEST),
} direction;

static inline square BB(file file, rank rank)
{
    return (rank << 3) + file;
}
static inline file BBfile(square square)
{
    return square & 7;
}
static inline rank BBrank(square square)
{
    return square >> 3;
}

void bitboard_init(void);

#endif  /* _BITBOARD_H */
