/* hq.c - hyperbola quintessence functions.
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

#include <brlib.h>

#include "chessdefs.h"
#include "board.h"
#include "bitboard.h"
#include "hq.h"


uchar bb_rank_attacks[64 * 8];

/**
 * hq_init() - init hyperbola quintessence attack bitboards
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
 * to save one operation in hq_moves().
 * TODO ? replace rank attack with this idea, mapping rank to diagonal ?
 * See http://timcooijmans.blogspot.com/2014/04/
 */
void hq_init()
{
    /* generate rank attacks, not handled by HQ
     */
    for (int occ = 0; occ < 64; ++occ) {
        for (int file = 0; file < 8; ++file) {
            int attacks = 0;

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
                    break;
            }
            bb_rank_attacks[(occ << 3) + file] = attacks;
        }
    }
}

/**
 * hq_rank_moves() - get rank moves for a sliding piece.
 * @pieces: occupation bitboard
 * @sq: piece square
 *
 * Rank attacks are not handled by HQ, so this function uses a pre-calculated
 * rank attacks table (@bb_rank_attacks).
 *
 * @Return: bitboard of @piece available pseudo-moves.
 */
bitboard_t hq_rank_moves(bitboard_t occ, square_t sq)
{
    u32 rank = sq & SQ_RANKMASK;
    u32 file = sq & SQ_FILEMASK;
    u64 o = (occ >> rank) & 0176;                 /* 01111110 clear bits 0 & 7 */
    return ((bitboard_t)bb_rank_attacks[(o << 2) + file]) << rank;
}

/**
 * hq_moves() - get hyperbola pseudo-moves for a sliding piece
 * @pieces: occupation bitboard
 * @sq: piece square
 * @mask: the appropriate mask (pre-calculated)
 *
 * This function can be used for files, diagonal, and anti-diagonal attacks.
 * @mask is the corresponding pre-calculated table (@bb_sqfile, @bb_sqdiag,
 * or @bb_sqanti).
 * See https://www.chessprogramming.org/Hyperbola_Quintessence for details.
 *
 * @Return: bitboard of piece available pseudo-moves.
 */
bitboard_t hq_moves(const bitboard_t pieces, const square_t sq,
                                   const bitboard_t mask)
{
    bitboard_t o = pieces & mask;
    bitboard_t r = bswap64(o);
    square_t  r_sq = FLIP_V(sq);

    return (         (o - 2 * BIT(sq)   )
            ^ bswap64(r - 2 * BIT(r_sq)))
        & mask;
}

/**
 * hq_file_moves() - get file pseudo-moves for a sliding piece.
 * @pieces: occupation bitboard
 * @sq: piece square
 *
 * @Return: bitboard of piece available pseudo-moves on its file.
 */
bitboard_t hq_file_moves(const bitboard_t occ, const square_t sq)
{
    return hq_moves(occ, sq, bb_sqfile[sq]);
}

/**
 * hq_diag_moves() - get diagonal pseudo-moves for a sliding piece.
 * @pieces: occupation bitboard
 * @sq: piece square
 *
 * @Return: bitboard of piece available pseudo-moves on its diagonal.
 */
bitboard_t hq_diag_moves(const bitboard_t occ, const square_t sq)
{
    return hq_moves(occ, sq, bb_sqdiag[sq]);
}

/**
 * hq_anti_moves() - get anti-diagonal pseudo-moves for a sliding piece.
 * @pieces: occupation bitboard
 * @sq: piece square
 *
 * @Return: bitboard of piece available pseudo-moves on its anti-diagonal.
 */
bitboard_t hq_anti_moves(const bitboard_t occ, const square_t sq)
{
    return hq_moves(occ, sq, bb_sqanti[sq]);
}

/**
 * hq_bishop_moves() - get bitboard of bishop pseudo-moves
 * @occ: occupation bitboard
 * @sq: bishop square
 *
 * @Return: bitboard of bishop available pseudo-moves.
 */
bitboard_t hq_bishop_moves(const bitboard_t occ, const square_t sq)
{
    return hq_diag_moves(occ, sq) | hq_anti_moves(occ, sq);
}

/**
 * hq_rook_moves() - get bitboard of rook pseudo-moves
 * @occ: occupation bitboard
 * @sq: rook square
 *
 * @Return: bitboard of rook available pseudo-moves.
 */
bitboard_t hq_rook_moves(const bitboard_t occ, const square_t sq)
{
    return hq_file_moves(occ, sq) | hq_rank_moves(occ, sq);
}

/**
 * hq_queen_moves() - get bitboard of queen pseudo-moves
 * @occ: occupation bitboard
 * @sq: queen square
 *
 * This function is a wrapper over @hyperbola_bishop_moves() and
 * @hyperbola_rook_moves().
 *
 * @Return: bitboard of queen available pseudo-moves.
 */
bitboard_t hq_queen_moves(const bitboard_t occ, const square_t sq)
{
    return hq_bishop_moves(occ, sq) | hq_rook_moves(occ, sq);
}
