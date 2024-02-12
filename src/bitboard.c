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
#include <stdarg.h>

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
static int bishop_vector[4] = {
    NORTH_EAST, SOUTH_EAST, SOUTH_WEST, NORTH_WEST
};
static int rook_vector[4] = {
    NORTH, EAST, SOUTH, WEST
};

bitboard_t sq_bb[SQUARE_MAX];
bitboard_t sq_bbrank[64], sq_bbfile[64], sq_bbdiag[64], sq_bbanti[64];

bitboard_t knight_attacks[64], king_attacks[64];
bitboard_t pawn_attacks[2];

/*  we will create only dest squares for A1-D4 square, then flip
 */

/**
 * raw_bitboard_print() - print simple bitboard representation
 * @bb: the bitboard
 * @tit: a string or NULL
 */
void bitboard_init(void)
{
    struct {
        union {
            struct {
                bitboard_t diag;
                bitboard_t anti;
                bitboard_t vert;
                bitboard_t hori;
            };
            bitboard_t all[4];
        };
    } tmpmasks[64] = {0};
    struct {
        int df, dr;
    } dirs[4] = {
        { 0,  1 },                                /* vertical/file */
        { 1,  0 },                                /* horizontal/rank */
        { 1,  1 },                                /* diagonal */
        { 1, -1 },                                /* antidiagonal */
    } ;

    square_t sq;
    int dst, dst_f, dst_r;

    /* square to bitboard */
    for (sq = A1; sq <= H8; ++sq)
        sq_bb[sq] = mask(sq);

    /* square to rank/file/diagonal/antidiagonal */
    for (sq = 0; sq < 64; ++sq) {
        int r = sq_rank(sq), f = sq_file(sq);
        for (int mult = -7; mult < 8; ++mult) {
            for (int dir = 0; dir < 4; ++dir) {
                dst_f = f + mult * dirs[dir].df;
                dst_r = r + mult * dirs[dir].dr;
                if (sq_coord_ok(dst_f) && sq_coord_ok(dst_r)) {
                    dst = sq_make(dst_f, dst_r);
                    tmpmasks[sq].all[dir] |= mask(dst);
                }
            }
        }
    }
    for (sq = 0; sq < 64; ++sq) {
        sq_bbfile[sq] = tmpmasks[sq].all[0];
        sq_bbrank[sq] = tmpmasks[sq].all[1];
        sq_bbdiag[sq] = tmpmasks[sq].all[2];
        sq_bbanti[sq] = tmpmasks[sq].all[3];
    }

    /* knight and king attacks */
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
 * @title: a string or NULL
 * @bitboard: the bitboard
 */
void bitboard_print(const char *title, const bitboard_t bitboard)
{
    //char c = p? p: 'X';
    if (title)
        printf("%s\n", title);
    for (rank_t r = RANK_8; r >= RANK_1; --r) {
        printf("%d ", r + 1);
        for (file_t f = FILE_A; f <= FILE_H; ++f) {
            printf(" %c", bitboard & sq_bb[sq_make(f, r)] ? 'X': '.');
        }
        printf("\n");
    }
    printf("   a b c d e f g h\n");
    return;
}

/**
 * bitboard_print_multi() - print multiple bitboards horizontally
 * @title: a string or NULL
 * @n: number of bitboards
 * @bb_ptr...: pointers to bitboards
 *
 * @n is the number of bitboards to print. If @n -s > 8, it is reduced to 8;
 */
void bitboard_print_multi(const char *title, const int n, ...)
{
    bitboard_t bb[16];
    int i;
    va_list ap;

    va_start(ap, n);
    for (i = 0; i < n; ++i) {
        bb[i] = va_arg(ap, bitboard_t);
    }
    va_end(ap);
    //char c = p? p: 'X';
    if (title)
        printf("%s\n", title);

    for (rank_t r = RANK_8; r >= RANK_1; --r) {
        for (i = 0; i < n; ++i) {
            printf("%d ", r + 1);
            for (file_t f = FILE_A; f <= FILE_H; ++f) {
                printf(" %c", bb[i] & sq_bb[sq_make(f, r)] ? 'X': '.');
            }
            printf("    ");
        }
        printf("\n");
    }
    for (i = 0; i < n; ++i) {
        printf("   a b c d e f g h");
        printf("    ");
    }
    printf("\n");
    return;
}
