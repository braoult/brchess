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
#include "piece.h"
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

bitboard_t bb_sq[SQUARE_MAX];
bitboard_t bb_rank[64], bb_file[64], bb_diagonal[64], bb_antidiagonal[64];

bitboard_t bb_knight[64], bb_king[64];
bitboard_t bb_pawn_push[2][64], bb_bpawn_attack[2][64], bb_pawn_ep[2][64];

/**
 * bitboard_init() - initialize general bitboards
 *
 * Generate the following bitboards :
 *   bb_sq[64]: square to bitboard
 *   bb_rank[64]: square to rank
 *   bb_file[64]: square to file
 *   bb_diagonal[64]: square to diagonal
 *   bb_antidiagonal[64]: square to antidiagonal
 *
 * And the following pseudo move masks:
 *   bb_knight[64]: knight moves
 *   bb_king[64]: king moves
 *   bb_pawn[2][64]: white pawn moves (not attacks)
 *   bb_pawn_att[2][64]: white pawn attacks
 */
void bitboard_init(void)
{
    /*  for each square, the 4 masks: file, rank, diagonal, antidiagonal */
    struct { int df, dr; } dirs[4] = {
        { 0,  1 },                                /* vertical/file */
        { 1,  0 },                                /* horizontal/rank */
        { 1,  1 },                                /* diagonal */
        { 1, -1 },                                /* antidiagonal */
    } ;
    bitboard_t tmpbb[64][4] = { 0 };

    /* 1) square to bitboard */
    for (square_t sq = A1; sq <= H8; ++sq)
        bb_sq[sq] = mask(sq);

    /* 2) square to rank/file/diagonal/antidiagonal */
    for (square_t sq = 0; sq < 64; ++sq) {
        int r = sq_rank(sq), f = sq_file(sq);
        for (int mult = -7; mult < 8; ++mult) {
            for (int dir = 0; dir < 4; ++dir) {
                int dst_f = f + mult * dirs[dir].df;
                int dst_r = r + mult * dirs[dir].dr;
                if (sq_coord_ok(dst_f) && sq_coord_ok(dst_r)) {
                    int dst = sq_make(dst_f, dst_r);
                    tmpbb[sq][dir] |= mask(dst);
                }
            }
        }
    }
    for (square_t sq = 0; sq < 64; ++sq) {
        bb_file[sq] = tmpbb[sq][0];
        bb_rank[sq] = tmpbb[sq][1];
        bb_diagonal[sq] = tmpbb[sq][2];
        bb_antidiagonal[sq] = tmpbb[sq][3];
    }

    /* 3) knight and king moves */
    for (square_t sq = A1; sq <= H8; ++sq) {
        for (int vec = 0; vec < 8; ++vec) {
            int dst = sq + knight_vector[vec];
            if (sq_ok(dst)) {
                if (sq_dist(dst, sq) == 2) {
                    bb_knight[sq] |= bb_sq[dst];
                }
            }
            dst = sq + king_vector[vec];
            if (sq_ok(dst)) {
                if (sq_dist(dst, sq) == 1) {
                    bb_king[sq] |= bb_sq[dst];
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
            printf(" %c", bitboard & bb_sq[sq_make(f, r)] ? 'X': '.');
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
void bitboard_print_multi(const char *title, int n, ...)
{
    bitboard_t bb[8];
    va_list ap;

    n = min(n, 8);

    va_start(ap, n);
    for (int i = 0; i < n; ++i) {                 /* save all bitboards */
        bb[i] = va_arg(ap, bitboard_t);
    }
    va_end(ap);

    if (title)
        printf("%s\n", title);

    for (rank_t r = RANK_8; r >= RANK_1; --r) {
        for (int i = 0; i < n; ++i) {
            printf("%d ", r + 1);
            for (file_t f = FILE_A; f <= FILE_H; ++f) {
                printf(" %c", bb[i] & bb_sq[sq_make(f, r)] ? 'X': '.');
            }
            printf("    ");
        }
        printf("\n");
    }
    for (int i = 0; i < n; ++i) {
        printf("   a b c d e f g h");
        printf("    ");
    }
    printf("\n");
    return;
}

/**
 * bitboard_rank_sprint() - print an u8 rank binary representation
 * @str: the destination string
 * @bb8: the uchar to print
 */
char *bitboard_rank_sprint(char *str, const uchar bb8)
{
    for (file_t f = FILE_A; f <= FILE_H; ++f) {
        *(str+f) = bb8 & mask(f) ? '1': '.';
    }
    //printf(" 0 1 2 3 4 5 6 7\n");
    //printf("\n");
    return str;
}
