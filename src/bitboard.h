/* bitboard.h - bitboard definitions.
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

#ifndef BITBOARD_H
#define BITBOARD_H

#include <bits.h>

#include "chessdefs.h"
#include "piece.h"

enum bb_square {
    A1 = 1UL <<  0, B1 = 1UL <<  1, C1 = 1UL <<  2, D1 = 1UL <<  3,
    E1 = 1UL <<  4, F1 = 1UL <<  5, G1 = 1UL <<  6, H1 = 1UL <<  7,

    A2 = 1UL <<  8, B2 = 1UL <<  9, C2 = 1UL << 10, D2 = 1UL << 11,
    E2 = 1UL << 12, F2 = 1UL << 13, G2 = 1UL << 14, H2 = 1UL << 15,

    A3 = 1UL << 16, B3 = 1UL << 17, C3 = 1UL << 18, D3 = 1UL << 19,
    E3 = 1UL << 20, F3 = 1UL << 21, G3 = 1UL << 22, H3 = 1UL << 23,

    A4 = 1UL << 24, B4 = 1UL << 25, C4 = 1UL << 26, D4 = 1UL << 27,
    E4 = 1UL << 28, F4 = 1UL << 29, G4 = 1UL << 30, H4 = 1UL << 31,

    A5 = 1UL << 32, B5 = 1UL << 33, C5 = 1UL << 34, D5 = 1UL << 35,
    E5 = 1UL << 36, F5 = 1UL << 37, G5 = 1UL << 38, H5 = 1UL << 39,

    A6 = 1UL << 40, B6 = 1UL << 41, C6 = 1UL << 42, D6 = 1UL << 43,
    E6 = 1UL << 44, F6 = 1UL << 45, G6 = 1UL << 46, H6 = 1UL << 47,

    A7 = 1UL << 48, B7 = 1UL << 49, C7 = 1UL << 50, D7 = 1UL << 51,
    E7 = 1UL << 52, F7 = 1UL << 53, G7 = 1UL << 54, H7 = 1UL << 55,

    A8 = 1UL << 56, B8 = 1UL << 57, C8 = 1UL << 58, D8 = 1UL << 59,
    E8 = 1UL << 60, F8 = 1UL << 61, G8 = 1UL << 62, H8 = 1UL << 63,
};

enum bb_files {
    F_1 = A1 | A2 | A3 | A4 | A5 | A6 | A7 | A8,
    F_2 = B1 | B2 | B3 | B4 | B5 | B6 | B7 | B8,
    F_3 = C1 | C2 | C3 | C4 | C5 | C6 | C7 | C8,
    F_4 = D1 | D2 | D3 | D4 | D5 | D6 | D7 | D8,
    F_5 = E1 | E2 | E3 | E4 | E5 | E6 | E7 | E8,
    F_6 = F1 | F2 | F3 | F4 | F5 | F6 | F7 | F8,
    F_7 = G1 | G2 | G3 | G4 | G5 | G6 | G7 | G8,
    F_8 = H1 | H2 | H3 | H4 | H5 | H6 | H7 | H8,
};

enum bb_ranges {
    R_1 = A1 | B1 | C1 | D1 | E1 | F1 | G1 | H1,
    R_2 = A2 | B2 | C2 | D2 | E2 | F2 | G2 | H2,
    R_3 = A3 | B3 | C3 | D3 | E3 | F3 | G3 | H3,
    R_4 = A4 | B4 | C4 | D4 | E4 | F4 | G4 | H4,
    R_5 = A5 | B5 | C5 | D5 | E5 | F5 | G5 | H5,
    R_6 = A6 | B6 | C6 | D6 | E6 | F6 | G6 | H6,
    R_7 = A7 | B7 | C7 | D7 | E7 | F7 | G7 | H7,
    R_8 = A8 | B8 | C8 | D8 | E8 | F8 | G8 | H8,
};

#endif  /* BOARD_H */
