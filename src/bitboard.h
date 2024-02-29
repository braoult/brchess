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

typedef u64 bitboard_t;

/* mapping square -> bitboard */
extern bitboard_t bb_sq[64];
/* squares between sq1 and sq2, exclusing both */
extern bitboard_t bb_between_excl[64][64];
/* squares between sq1 and sq2, including sq2 */
extern bitboard_t bb_between[64][64];

/* bb_rank[64]: square to rank
 * bb_file[64]: square to file
 * bb_diag[64]: square to diagonal
 * bb_anti[64]: square to antidiagonal
 */
extern bitboard_t bb_rank[64], bb_file[64], bb_diag[64], bb_anti[64];
/* knight and king moves */
extern bitboard_t bb_knight[64], bb_king[64];

enum {
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
};

enum {
    FILE_Abb = 0x0101010101010101ULL, FILE_Bbb = 0x0202020202020202ULL,
    FILE_Cbb = 0x0404040404040404ULL, FILE_Dbb = 0x0808080808080808ULL,
    FILE_Ebb = 0x1010101010101010ULL, FILE_Fbb = 0x2020202020202020ULL,
    FILE_Gbb = 0x4040404040404040ULL, FILE_Hbb = 0x8080808080808080ULL,

    RANK_1bb = 0x00000000000000ffULL, RANK_2bb = 0x000000000000ff00ULL,
    RANK_3bb = 0x0000000000ff0000ULL, RANK_4bb = 0x00000000ff000000ULL,
    RANK_5bb = 0x000000ff00000000ULL, RANK_6bb = 0x0000ff0000000000ULL,
    RANK_7bb = 0x00ff000000000000ULL, RANK_8bb = 0xff00000000000000ULL
};

/* https://www.chessprogramming.org/Flipping_Mirroring_and_Rotating#Rotation
 */
static __always_inline bitboard_t bb_rotate_90(bitboard_t b)
{
    return b;
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
/* pawn move (for single pawn) */
#define pawn_push_up(sq, c)       ((sq) + ((c) == WHITE ? NORTH:      SOUTH))
#define pawn_push_upleft(sq, c)   ((sq) + ((c) == WHITE ? NORTH_WEST: SOUTH_EAST))
#define pawn_push_upright(sq, c)  ((sq) + ((c) == WHITE ? NORTH_EAST: SOUTH_WEST))

extern bitboard_t bitboard_between_excl(square_t sq1, square_t sq2);
extern void bitboard_init(void);

extern bitboard_t bb_knight_moves(bitboard_t occ, square_t sq);
extern bitboard_t bb_king_moves(bitboard_t occ, square_t sq);

extern void bitboard_print(const char *title, const bitboard_t bitboard);
extern void bitboard_print_multi(const char *title, const int n, ...);
extern char *bitboard_rank_sprint(char *str, const uchar bb8);

#endif  /* _BITBOARD_H */
