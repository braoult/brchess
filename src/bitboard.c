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
#include "position.h"

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

bitboard_t bb_sq[64];
bitboard_t bb_rank[64], bb_file[64], bb_diag[64], bb_anti[64];
bitboard_t bb_between_excl[64][64];
bitboard_t bb_between[64][64];

bitboard_t bb_knight[64], bb_king[64];
bitboard_t bb_pawn_push[2][64], bb_bpawn_attack[2][64], bb_pawn_ep[2][64];

/**
 * bitboard_between_excl() - get bitboard of squares between two squares.
 * @sq1, @sq2: The two square_t squares
 *
 * From: http://www.talkchess.com/forum3/viewtopic.php?f=7&t=12499&start=14
 * This function may be used instead of bb_XXX arrays if cache pressure is high.
 *
 * @Return: bitboard_t, squares between @sq1 and @sq2 (excl. @sq1 and @sq2).
 */
bitboard_t bitboard_between_excl(square_t sq1, square_t sq2)
{
    const bitboard_t m1 = -1;
    const bitboard_t a2a7 = C64(0x0001010101010100);
    const bitboard_t b7h1 = C64(0x0002040810204080);
    bitboard_t btwn_bits, ray_bits;
    u32 rank_diff, file_diff, anti_diff, diag_diff;

    btwn_bits  =  (m1 << sq1) ^ (m1 << sq2);      /* includes sq1 and sq2 */
    rank_diff  = ((sq2 | 7) - sq1) >> 3,          /* signed */
    file_diff  =  (sq2 & 7) - (sq1 & 7);          /* signed */

    anti_diff  =  rank_diff  + file_diff;
    rank_diff  =  rank_diff  & 15;
    file_diff  =  file_diff  & 15;
    anti_diff  =  anti_diff  & 15;
    diag_diff  =  rank_diff  ^ file_diff;
    ray_bits   =  2 * ((rank_diff - 1) >> 26);

    ray_bits  |=  bswap64((m1 + diag_diff) & b7h1);
    ray_bits  |=  (m1 + anti_diff) & b7h1;
    ray_bits  |=  (m1 + file_diff) & a2a7;
    ray_bits  *=  btwn_bits  &  -btwn_bits;
    return ray_bits & btwn_bits;
}

/**
 * bitboard_init() - initialize general bitboards
 *
 * Generate the following bitboards :
 *   bb_sq[64]: square to bitboard
 *   bb_between_excl[64][64]: strict squares between two squares
 *   bb_between[64][64]: squares between two squares including second square
 *   bb_rank[64]: square to rank
 *   bb_file[64]: square to file
 *   bb_diagonal[64]: square to diagonal
 *   bb_antidiagonal[64]: square to antidiagonal
 *
 * And the following pseudo move masks:
 *   bb_knight[64]: knight moves
 *   bb_king[64]: king moves
 *
 */
void bitboard_init(void)
{
    /*  for each square, the 4 masks: file, rank, diagonal, antidiagonal */
    struct { int df, dr; } vecs[4] = {
        { 0,  1 },                                /* vertical/file */
        { 1,  0 },                                /* horizontal/rank */
        { 1,  1 },                                /* diagonal */
        { 1, -1 },                                /* antidiagonal */
    } ;
    bitboard_t tmpbb[64][4] = { 0 };

    /* 1) square to bitboard, and in-between-sq2-excluded */
    for (square_t sq1 = A1; sq1 <= H8; ++sq1) {
        bb_sq[sq1] = mask(sq1);
        for (square_t sq2 = A1; sq2 <= H8; ++sq2)
            bb_between_excl[sq1][sq2] = bitboard_between_excl(sq1, sq2);
    }

    /* 2) sq1-to-sq2 mask, sq2 included
     *    square to file/rank/dia/anti bitmaps
     */
    for (square_t sq = 0; sq < 64; ++sq) {
        file_t f = sq_file(sq);
        rank_t r = sq_rank(sq);
        for (int vec = 0; vec < 4; ++vec) {
            tmpbb[sq][vec] |= mask(sq_make(f, r));
            for (int dir = -1; dir <= 1; dir += 2) {
                file_t df = dir * vecs[vec].df, f2 = f + df;
                rank_t dr = dir * vecs[vec].dr, r2 = r + dr;
                bitboard_t mask_between = 0;
                while (sq_coord_ok(f2) && sq_coord_ok(r2)) {
                    square_t dest = sq_make(f2, r2);
                    tmpbb[sq][vec] |= mask(dest);
                    mask_between |= mask(dest);
                    bb_between[sq][dest] = mask_between;
                    f2 += df, r2 += dr;
                }
            }
        }
    }
    for (square_t sq = 0; sq < 64; ++sq) {
        bb_file[sq] = tmpbb[sq][0];
        bb_rank[sq] = tmpbb[sq][1];
        bb_diag[sq] = tmpbb[sq][2];
        bb_anti[sq] = tmpbb[sq][3];
    }

    /*
     * for (int i = 0; i < 64; ++i) {
     *     for (int j = 0; j < 64; ++j) {
     *         if (bb_between[i][j] != bb_between_excl[i][j]) {
     *             bitboard_t diff = bb_between_excl[i][j] ^ bb_between[i][j];
     *             int k = popcount64(bb_between[i][j]) -
     *                 popcount64(bb_between_excl[i][j]);
     *             printf("%s-%s diff=%d excl=%s   ",
     *                    sq_string(i), sq_string(j),
     *                    k,
     *                    sq_string(ctz64(diff)));
     *             if (k == 1 && ctz64(diff) == j)
     *                 printf("OK\n");
     *             else
     *                 printf("NOK\n");
     *         }
     *     }
     * }
     */

    /* 3) knight and king moves */
    for (square_t sq = A1; sq <= H8; ++sq) {
        //rank_t r1 = sq_rank(sq);
        //file_t f1 = sq_file(sq);
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

bitboard_t bb_knight_moves(bitboard_t notmine, square_t sq)
{
    return bb_knight[sq] & notmine;
}
bitboard_t bb_king_moves(bitboard_t notmine, square_t sq)
{
    return bb_king[sq] & notmine;
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
 * @n is the number of bitboards to print. If @n > 10, it is reduced to 10
 */
void bitboard_print_multi(const char *title, int n, ...)
{
    bitboard_t bb[8];
    va_list ap;

    n = min(n, 10);

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
