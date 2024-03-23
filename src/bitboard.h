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
#include "piece.h"

/* mapping square -> bitboard */
extern bitboard_t bb_sq[64];
/* squares between sq1 and sq2, exclusing both */
extern bitboard_t bb_between_excl[64][64];
/* squares between sq1 and sq2, including sq2 */
extern bitboard_t bb_between[64][64];

/**
 * bb_sqrank[64]: square to rank
 * bb_sqfile[64]: square to file
 * bb_sqdiag[64]: square to diagonal
 * bb_sqanti[64]: square to antidiagonal
 */
extern bitboard_t bb_sqrank[64], bb_sqfile[64], bb_sqdiag[64], bb_sqanti[64];

/* line (rank, file, diagonal or anti-diagonal) between two squares */
extern bitboard_t bb_line[64][64];

/* knight and king moves */
extern bitboard_t bb_knight[64], bb_king[64];


/* TODO (maybe C23?) when we can ensure an enum can be u64
 *
 * enum {
 *     A1bb = mask(A1), A2bb = mask(A2), A3bb = mask(A3), A4bb = mask(A4),
 *     A5bb = mask(A5), A6bb = mask(A6), A7bb = mask(A7), A8bb = mask(A8),
 *     B1bb = mask(B1), B2bb = mask(B2), B3bb = mask(B3), B4bb = mask(B4),
 *     B5bb = mask(B5), B6bb = mask(B6), B7bb = mask(B7), B8bb = mask(B8),
 *     C1bb = mask(C1), C2bb = mask(C2), C3bb = mask(C3), C4bb = mask(C4),
 *     C5bb = mask(C5), C6bb = mask(C6), C7bb = mask(C7), C8bb = mask(C8),
 *     D1bb = mask(D1), D2bb = mask(D2), D3bb = mask(D3), D4bb = mask(D4),
 *     D5bb = mask(D5), D6bb = mask(D6), D7bb = mask(D7), D8bb = mask(D8),
 *     E1bb = mask(E1), E2bb = mask(E2), E3bb = mask(E3), E4bb = mask(E4),
 *     E5bb = mask(E5), E6bb = mask(E6), E7bb = mask(E7), E8bb = mask(E8),
 *     F1bb = mask(F1), F2bb = mask(F2), F3bb = mask(F3), F4bb = mask(F4),
 *     F5bb = mask(F5), F6bb = mask(F6), F7bb = mask(F7), F8bb = mask(F8),
 *     G1bb = mask(G1), G2bb = mask(G2), G3bb = mask(G3), G4bb = mask(G4),
 *     G5bb = mask(G5), G6bb = mask(G6), G7bb = mask(G7), G8bb = mask(G8),
 *     H1bb = mask(H1), H2bb = mask(H2), H3bb = mask(H3), H4bb = mask(H4),
 *     H5bb = mask(H5), H6bb = mask(H6), H7bb = mask(H7), H8bb = mask(H8),
 * };
 *
 * enum {
 *     FILE_Abb = 0x0101010101010101ull, FILE_Bbb = 0x0202020202020202ull,
 *     FILE_Cbb = 0x0404040404040404ull, FILE_Dbb = 0x0808080808080808ull,
 *     FILE_Ebb = 0x1010101010101010ull, FILE_Fbb = 0x2020202020202020ull,
 *     FILE_Gbb = 0x4040404040404040ull, FILE_Hbb = 0x8080808080808080ull,
 *
 *     RANK_1bb = 0x00000000000000ffull, RANK_2bb = 0x000000000000ff00ull,
 *     RANK_3bb = 0x0000000000ff0000ull, RANK_4bb = 0x00000000ff000000ull,
 *     RANK_5bb = 0x000000ff00000000ull, RANK_6bb = 0x0000ff0000000000ull,
 *     RANK_7bb = 0x00ff000000000000ull, RANK_8bb = 0xff00000000000000ull
 * };
 */

/* generated with bash:
 * R="ABCDEFGH"
 * F="12345678"
 * for i in {0..63}; do
 *   printf "#define  %c%cbb %#018llxull\n" ${R:i/8:1} ${F:i%8:1} $((1 << i))
 * done
 */
#define A1bb     0x0000000000000001ull
#define A2bb     0x0000000000000002ull
#define A3bb     0x0000000000000004ull
#define A4bb     0x0000000000000008ull
#define A5bb     0x0000000000000010ull
#define A6bb     0x0000000000000020ull
#define A7bb     0x0000000000000040ull
#define A8bb     0x0000000000000080ull
#define B1bb     0x0000000000000100ull
#define B2bb     0x0000000000000200ull
#define B3bb     0x0000000000000400ull
#define B4bb     0x0000000000000800ull
#define B5bb     0x0000000000001000ull
#define B6bb     0x0000000000002000ull
#define B7bb     0x0000000000004000ull
#define B8bb     0x0000000000008000ull
#define C1bb     0x0000000000010000ull
#define C2bb     0x0000000000020000ull
#define C3bb     0x0000000000040000ull
#define C4bb     0x0000000000080000ull
#define C5bb     0x0000000000100000ull
#define C6bb     0x0000000000200000ull
#define C7bb     0x0000000000400000ull
#define C8bb     0x0000000000800000ull
#define D1bb     0x0000000001000000ull
#define D2bb     0x0000000002000000ull
#define D3bb     0x0000000004000000ull
#define D4bb     0x0000000008000000ull
#define D5bb     0x0000000010000000ull
#define D6bb     0x0000000020000000ull
#define D7bb     0x0000000040000000ull
#define D8bb     0x0000000080000000ull
#define E1bb     0x0000000100000000ull
#define E2bb     0x0000000200000000ull
#define E3bb     0x0000000400000000ull
#define E4bb     0x0000000800000000ull
#define E5bb     0x0000001000000000ull
#define E6bb     0x0000002000000000ull
#define E7bb     0x0000004000000000ull
#define E8bb     0x0000008000000000ull
#define F1bb     0x0000010000000000ull
#define F2bb     0x0000020000000000ull
#define F3bb     0x0000040000000000ull
#define F4bb     0x0000080000000000ull
#define F5bb     0x0000100000000000ull
#define F6bb     0x0000200000000000ull
#define F7bb     0x0000400000000000ull
#define F8bb     0x0000800000000000ull
#define G1bb     0x0001000000000000ull
#define G2bb     0x0002000000000000ull
#define G3bb     0x0004000000000000ull
#define G4bb     0x0008000000000000ull
#define G5bb     0x0010000000000000ull
#define G6bb     0x0020000000000000ull
#define G7bb     0x0040000000000000ull
#define G8bb     0x0080000000000000ull
#define H1bb     0x0100000000000000ull
#define H2bb     0x0200000000000000ull
#define H3bb     0x0400000000000000ull
#define H4bb     0x0800000000000000ull
#define H5bb     0x1000000000000000ull
#define H6bb     0x2000000000000000ull
#define H7bb     0x4000000000000000ull
#define H8bb     0x8000000000000000ull

#define FILE_Abb 0x0101010101010101ull
#define FILE_Bbb 0x0202020202020202ull
#define FILE_Cbb 0x0404040404040404ull
#define FILE_Dbb 0x0808080808080808ull
#define FILE_Ebb 0x1010101010101010ull
#define FILE_Fbb 0x2020202020202020ull
#define FILE_Gbb 0x4040404040404040ull
#define FILE_Hbb 0x8080808080808080ull

#define RANK_1bb 0x00000000000000ffull
#define RANK_2bb 0x000000000000ff00ull
#define RANK_3bb 0x0000000000ff0000ull
#define RANK_4bb 0x00000000ff000000ull
#define RANK_5bb 0x000000ff00000000ull
#define RANK_6bb 0x0000ff0000000000ull
#define RANK_7bb 0x00ff000000000000ull
#define RANK_8bb 0xff00000000000000ull


/*
static __always_inline bitboard_t bb_rank(int rank)
{
    return RANK_1bb << (rank * 8);
}
static __always_inline bitboard_t bb_rel_rank(int rank, color)
{
    return RANK_1bb << (rank * 8);
}
static __always_inline bitboard_t bb_file(int file)
{
    return FILE_Abb << file;
}
*/

#define bb_rank(r)        ((u64) RANK_1bb << ((r) * 8))
#define BB_FILE(f)        ((u64) FILE_Abb << (f))

#define bb_rel_rank(r, c) bb_rank(sq_rel_rank(r, c))

//#define BB_REL_RANK(r, c) (RANK_1bb << (SQ_REL_RANK(r, c) * 8))
//#define BB_REL_FILE(f, c) (FILE_Abb << (SQ_REL_RANK((f), (c))))

/**
 * bb_sq_aligned() - check if two squares are aligned (same file or rank).
 * @sq1, @sq2:  the two squares.
 *
 * @return: true if @sq1 and @sq2 are on same line, false otherwise.
 */
static __always_inline bool bb_sq_aligned(square_t sq1, square_t sq2)
{
    return bb_line[sq1][sq2];
}

/**
 * bb_sq_aligned3() - check if 3 squares are aligned (same file or rank).
 * @sq1, @sq2, @sq3:  the three squares.
 *
 * @return: true if @sq1, @sq2, and @sq3 are aligned, false otherwise.
 */
static __always_inline bool bb_sq_aligned3(square_t sq1, square_t sq2, square_t sq3)
{
    return bb_line[sq1][sq2] & mask(sq3);
}

/**
 * bb_sq_between() - check if a square is between two squares
 * @sq:  the possibly "in-between" square
 * @sq1: square 1
 * @sq2: square 2
 *
 * @return: bitboard of @betw if between @sq1 and @sq2.
 */
static __always_inline bitboard_t bb_sq_between(square_t sq, square_t sq1, square_t sq2)
{
    return bb_between_excl[sq1][sq2] & mask(sq);
}

/* TODO: when OK, replace with macros */
static __always_inline bitboard_t shift_n(const bitboard_t bb)
{
    return bb << NORTH;
}
static __always_inline bitboard_t shift_ne(const bitboard_t bb)
{
    return (bb & ~FILE_Hbb) << NORTH_EAST;
}
static __always_inline bitboard_t shift_e(const bitboard_t bb)
{
    return (bb & ~FILE_Hbb) << EAST;
}
static __always_inline bitboard_t shift_se(const bitboard_t bb)
{
    return (bb & ~FILE_Hbb) >> -SOUTH_EAST;
}
static __always_inline bitboard_t shift_s(const bitboard_t bb)
{
    return bb >> -SOUTH;
}
static __always_inline bitboard_t shift_sw(const bitboard_t bb)
{
    return (bb & ~FILE_Abb) >> -SOUTH_WEST;
}
static __always_inline bitboard_t shift_w(const bitboard_t bb)
{
    return (bb & ~FILE_Abb) >> -WEST;
}
static __always_inline bitboard_t shift_nw(const bitboard_t bb)
{
    return (bb & ~FILE_Abb) << NORTH_WEST;
}

#define pawn_up_value(c)          ((c) == WHITE ? 8: -8)
/* pawn moves/attacks (for bitboards) */
#define pawn_shift_up(bb, c)      ((c) == WHITE ? shift_n(bb): shift_s(bb))
#define pawn_shift_upleft(bb, c)  ((c) == WHITE ? shift_nw(bb): shift_se(bb))
#define pawn_shift_upright(bb, c) ((c) == WHITE ? shift_ne(bb): shift_sw(bb))
/* pawn move (for single pawn) - NO SQUARE CONTROL HERE !
 * Need to make functions with control instead.
 */
#define pawn_push_up(sq, c)       ((sq) + ((c) == WHITE ? NORTH:      SOUTH))
#define pawn_push_upleft(sq, c)   ((sq) + ((c) == WHITE ? NORTH_WEST: SOUTH_EAST))
#define pawn_push_upright(sq, c)  ((sq) + ((c) == WHITE ? NORTH_EAST: SOUTH_WEST))

extern bitboard_t bitboard_between_excl(square_t sq1, square_t sq2);
extern void bitboard_init(void);

extern bitboard_t bb_knight_moves(bitboard_t occ, square_t sq);
extern bitboard_t bb_king_moves(bitboard_t occ, square_t sq);

extern void bb_print(const char *title, const bitboard_t bitboard);
extern void bb_print_multi(const char *title, const int n, ...);
extern char *bb_rank_sprint(char *str, const uchar bb8);
extern char *bb_sq2str(const bitboard_t bb, char *str, int len);

#endif  /* _BITBOARD_H */
