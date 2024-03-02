/* board.h - 8x8 board definitions.
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

#ifndef _BOARD_H
#define _BOARD_H

#include "brlib.h"

#include "chessdefs.h"
#include "piece.h"
#include "bitboard.h"

/* a square is defined as
 * rrrfff
 */
#define SQ_FILEMASK (007)                         /* warning, octal */
#define SQ_RANKMASK (070)


/* flip a 0-63 square:
 * Vertical:   G8 (62) becomes G1 (6)
 * Horizontal: G8 (62) becomes B8 (57)
 */
#define FLIP_V(sq)      ((sq) ^ 56)
#define FLIP_H(sq)      ((sq) ^ 7)
#define FLIP_HV(sq)     ((sq) ^ 63)               /* FLIP_V ^ FLIP_H */

/* TODO: revert to macros after bitboard migration */
static __always_inline square_t sq_make(file_t file, rank_t rank)
{
    return (rank << 3) + file;
}
static __always_inline file_t sq_file(square_t square)
{
    return square & SQ_FILEMASK;
}
static __always_inline rank_t sq_rank(square_t square)
{
    return square >> 3;
}

#define sq_ok(sq)       ((sq) >= A1 && (sq) <= H8)
#define sq_coord_ok(c)  ((c) >= 0 && (c) < 8)

/* Chebyshev distance: max( |r2 - r1|, |f2 - f1| )
 * See: https://www.chessprogramming.org/Distance
 */
#define sq_dist(sq1, sq2) (max(abs(sq_file(sq2) - sq_file(sq1)),  \
                               abs(sq_rank(sq2) - sq_rank(sq1))))
/* Manhattan distance: |r2 - r1| + |f2 - f1|
 */
#define sq_manh(sq1, sq2) (abs(sq_file(sq2) - sq_file(sq1)) +     \
                           abs(sq_rank(sq2) - sq_rank(sq1)))

extern const char *sq_to_string(const square_t sq);
extern square_t sq_from_string(const char *sq_string);

extern void board_print(piece_t *board);
extern void board_print_mask(piece_t *board, bitboard_t mask);

#endif  /* _BOARD_H */
