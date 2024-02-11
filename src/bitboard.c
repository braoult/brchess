/* bitboard.c - bitboard functions.
 *
 * Copyright (C) 2024 Bruno Raoult ("br")
 * Licensed under the GNU General Public License v3.0 or later.
 * Some rights reserved. See COPYING.
 *
 * You should have received a copy of the GNU General Public License along with this
 * program. If not, see <https://www.gnu.org/licenses/gpl-3.0-standalone.html>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later <https://spdx.org/licenses/GPL-3.0-or-later.html>
 *
 */

#include <stdio.h>

#include "brlib.h"

#include "chessdefs.h"
#include "board.h"
#include "bitboard.h"

/*  vectors are clockwise from N */
static int knight_vector[] = {
    NORTH_EAST + NORTH, NORTH_EAST + EAST,
    SOUTH_EAST + EAST,  SOUTH_EAST + SOUTH,
    SOUTH_WEST + SOUTH, SOUTH_WEST + WEST,
    NORTH_WEST + WEST,  NORTH_WEST + NORTH
};
static int king_vector[8] = {
    NORTH, NORTH_EAST, EAST, SOUTH_EAST,
    SOUTH, SOUTH_WEST, WEST, NORTH_WEST
};

bitboard_t sq_bb[SQUARE_MAX];
bitboard_t knight_attacks[64], king_attacks[64];

/*  we will create only dest squares for A1-D4 square, then flip
 */

/**
 * raw_bitboard_print() - print simple bitboard representation
 * @bb: the bitboard
 * @tit: a string or NULL
 */
void bitboard_init(void)
{
    square_t sq, dst;

    for (sq = A1; sq <= H8; ++sq)
        sq_bb[sq] = mask(sq);

    for (sq = A1; sq <= H8; ++sq) {
        for (int vec = 0; vec < 8; ++vec) {
            dst = sq + knight_vector[vec];
            if (sq_ok(dst)) {
                if (sq_dist(dst, sq) == 2) {
                    knight_attacks[sq] |= sq_bb[dst];
                }
            }
            dst = sq + king_vector[vec];
            if (sq_ok(dst)) {
                if (sq_dist(dst, sq) == 1) {
                    king_attacks[sq] |= sq_bb[dst];
                }
            }
        }
    }
}

/**
 * bitboard_print() - print simple bitboard representation
 * @bb: the bitboard
 * @tit: a string or NULL
 */
void bitboard_print(const bitboard_t bb, const char *tit)
{
    //char c = p? p: 'X';
    if (tit)
        printf("%s\n", tit);
    for (rank_t r = RANK_8; r >= RANK_1; --r) {
        printf("%d ", r);
        for (file_t f = FILE_A; f <= FILE_H; ++f) {
            printf(" %c", bb & sq_bb[sq_make(f, r)] ? 'X': '.');
        }
        printf("\n");
    }
    printf("\n   A B C D E F G H\n");
    return;
}
