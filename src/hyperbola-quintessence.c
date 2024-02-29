/* hyperbola-quintessence.c - hyperbola quintessence functions.
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
#include "hyperbola-quintessence.h"


uchar bb_rank_attacks[64 * 8];

/**
 * hyperbola_init() - init hyperbola quintessence attack bitboards
 *
 * See: https://www.chessprogramming.org/Kindergarten_Bitboards
 * and  https://www.chessprogramming.org/Hyperbola_Quintessence
 *
 *
 * Rank attacks table:
 * bb_rank_hyperbola[512 = 9 bits], indexed by oooooofff where:
 *   - O = oooooo: occupation of inner 6 bits on rank
 *   - F = fff:    file of sliding piece
 * The index is built as (oooooo << 3 + fff), = ((O << 3) + F), where:
 *   - O = all combinations of 6 bits (loop from 0 to 64)
 *   - F = all files (loop from 0 to 7)
 * To retrieve the index, given an 8 bits mask M=XooooooX, and a file F=fff,
 * we get O from M with:
 *   1) remove bits 'X' (O = M & 01111110)
 *   2) shift left result 2 more bits, as bit 0 is unused and already cleared:
 *      (O <<= 2)
 *
 * TODO ? create masks excluding slider (eg. bb_diag ^ bb_sq[square]),
 * to save one operation in hyperbola_moves().
 * TODO ? replace rank attack with this idea, mapping rank to diagonal ?
 * See http://timcooijmans.blogspot.com/2014/04/
 */
void hyperbola_init()
{
    /* generate rank attacks, not handled by HQ
     */
    for (int occ = 0; occ < 64; ++occ) {
        for (int file = 0; file < 8; ++file) {
            int attacks = 0;
            //int o = mask << 1;                    /* skip right square */

            /* set f left attacks */
            for (int slide = file - 1; slide >= 0; --slide) {
                int b = bb_sq[slide];             /* bit to consider */
                attacks |= b;                     /* add to attack mask */
                if ((occ << 1) & b)               /* piece on b, we stop */
                    break;
            }
            /* set f right attacks */
            for (int slide = file + 1; slide < 8; ++slide) {
                int b = bb_sq[slide];
                attacks |= b;
                if ((occ << 1) & b)               /* piece on b, we stop */
                    //if ((o & b) == b)
                    break;
            }
            bb_rank_attacks[(occ << 3) + file] = attacks;

            //if (((occ << 3) + file) == 171) {
            //char str[64], str2[64];
               //printf("mask=%x=%s file=%d att=%x=%s\n",
               //       occ, bitboard_rank_sprint(str, occ), file,
               //       attacks, bitboard_rank_sprint(str2, attacks));
            //}
        }
    }
}

/**
 * hyperbola_rank_moves() - generate rank moves for a sliding piece.
 * @pieces: occupation bitboard
 * @sq: piece square
 *
 * Rank attacks are not handled by HQ, so we do it with a
 *
 * @Return: The moves mask for piece
 */
static bitboard_t hyperbola_rank_moves(bitboard_t occ, square_t sq)
{
    u32 rank = sq & SQ_RANKMASK;
    u32 file = sq & SQ_FILEMASK;
    u64 o = (occ >> rank) & 0176;                 /* 01111110 clear bits 0 & 7 */
    //char zob[128], zob2[128];
    //printf("rank_moves: occ=%lx=%s file=%d o=%lx=%s index=%ld=%ld attack=%lx=%s\n", occ,
    //       bitboard_rank_sprint(zob, occ), file, o,
    //       bitboard_rank_sprint(zob, o), (o << 2) + file, (o * 4) + file,
    //       (bitboard_t)bb_rank_attacks[(o << 2) + file] << rank,
    //       bitboard_rank_sprint(zob2, (bitboard_t)bb_rank_attacks[(o << 2) + file] << rank));
    return ((bitboard_t)bb_rank_attacks[(o << 2) + file]) << rank;
}

/**
 * hyperbola_moves() - generate hyperbola moves mask for a given sliding piece
 * @pieces: occupation bitboard
 * @sq: piece square
 * @mask: mask considered
 *
 * See https://www.chessprogramming.org/Hyperbola_Quintessence
 *
 * @Return: The moves mask for piece
 */
static bitboard_t hyperbola_moves(const bitboard_t pieces, const square_t sq,
                                   const bitboard_t mask)
{
    bitboard_t o = pieces & mask;
    bitboard_t r = bswap64(o);
    square_t  r_sq = FLIP_V(sq);

    return (         (o - 2 * mask(sq)   )
            ^ bswap64(r - 2 * mask(r_sq)))
        & mask;
}

static bitboard_t hyperbola_file_moves(bitboard_t occ, square_t sq)
{
    return hyperbola_moves(occ, sq, bb_file[sq]);
}

static bitboard_t hyperbola_diag_moves(bitboard_t occ, square_t sq)
{
    return hyperbola_moves(occ, sq, bb_diag[sq]);
}

static bitboard_t hyperbola_anti_moves(bitboard_t occ, square_t sq)
{
    return hyperbola_moves(occ, sq, bb_anti[sq]);
}

bitboard_t hyperbola_bishop_moves(bitboard_t occ, square_t sq)
{
    return hyperbola_diag_moves(occ, sq) | hyperbola_anti_moves(occ, sq);
}

bitboard_t hyperbola_rook_moves(bitboard_t occ, square_t sq)
{
    return hyperbola_file_moves(occ, sq) | hyperbola_rank_moves(occ, sq);
}

bitboard_t hyperbola_queen_moves(bitboard_t occ, square_t sq)
{
    return hyperbola_bishop_moves(occ, sq) | hyperbola_rook_moves(occ, sq);
}
