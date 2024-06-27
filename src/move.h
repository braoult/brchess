/* move.h - move management.
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

#ifndef MOVE_H
#define MOVE_H

#include "chessdefs.h"
#include "piece.h"
#include "board.h"

/* move structure:
 * 1  1 1 1 1
 * 8  5 4 2 1    6 5    0
 * FFFF ppp tttttt ffffff
 *
 * bits    len off range         type      mask        get  desc
 * ffffff    6   0   0-5     square_t       077        &077 from
 * tttttt    6   6  6-11     square_t     07700 (>>6)  &077 to
 * ppp       3  12 12-14 piece_type_t    070000 (>>12) &07  promoted
 * FFF       3  15 15-17 move_flags_t   0700000 (>>15) &07  flags
 */
typedef u32 move_t;

enum {
    M_OFF_FROM     = 0,
    M_OFF_TO       = 6,
    M_OFF_PROMOTED = 12,
    M_OFF_FLAGS    = 15
};

typedef enum {
    M_PROMOTION  =  070000,
    M_FLAGS_MASK = 0700000,
    M_ENPASSANT  = (1 << M_OFF_FLAGS),
    M_CASTLE     = (2 << M_OFF_FLAGS),            /* maybe only one ? */
//  M_CHECK      = (3 << M_OFF_FLAGS)             /* maybe unknown/useless ? */
} move_flags_t;

/* special move_t values */
#define MOVE_NULL    0                            /* hack: from = to = A1 */
#define MOVE_NONE    07777                        /* hack: from = to = H8 */
#define MOVE_NO_MOVE 01010                        /* hack: from = to = A2 */

#define move_set_flags(move, flags) ((move) | (flags))
#define move_flags(move)            ((move) & M_FLAGS_MASK)

#define is_promotion(m)             ((m) &  M_PROMOTION)
#define is_enpassant(m)             (move_flags(m) == M_ENPASSANT)
#define is_castle(m)                (move_flags(m) == M_CASTLE)
#define is_check(m)                 (move_flags(m) == M_CHECK)

#define MOVES_MAX   256

typedef struct __movelist_s {
    move_t move[MOVES_MAX];
    int nmoves;                                   /* total moves (fill) */
} movelist_t;

static inline square_t move_from(move_t move)
{
    return move & 077;
}

static inline square_t move_to(move_t move)
{
    return (move >> M_OFF_TO) & 077;
}

static inline piece_type_t move_promoted(move_t move)
{
    return (move >> M_OFF_PROMOTED) & 07;
}

/*
 * static inline piece_type_t move_captured(move_t move)
 * {
 *     return (move >> M_OFF_CAPTURED) & 07;
 * }
 */

static inline move_t move_make(square_t from, square_t to)
{
    return (to << M_OFF_TO) | from;
}

static inline move_t move_make_flags(square_t from, square_t to, move_flags_t flags)
{
    return (to << M_OFF_TO) | from | flags;
    //move_set_flags(move_make(from, to), flags);
}

/*
 * static inline move_t move_make_capture(square_t from, square_t to)
 * {
 *     return move_make_flags(from, to, M_CAPTURE);
 * }
 */

static inline move_t move_make_enpassant(square_t from, square_t to)
{
    return move_make_flags(from, to, M_ENPASSANT);
}

static inline move_t move_make_promote(square_t from, square_t to,
                                       piece_type_t promoted)
{
    return move_make(from, to) | (promoted << M_OFF_PROMOTED);
}

/*
 * static inline move_t move_make_promote_capture(square_t from, square_t to,
 *                                                piece_type_t promoted)
 * {
 *     return move_make_promote(from, to, promoted) | M_CAPTURE;
 * }
 */

/*
 * static inline move_t move_set_captured(move_t move, piece_type_t captured)
 * {
 *     return move | (captured << M_OFF_CAPTURED);
 * }
 */

/* moves_print flags
 */
#define M_PR_CAPT      0x01
#define M_PR_NCAPT     0x02
#define M_PR_NUM       0x04
#define M_PR_NL        0x08
#define M_UCI          0x10                       /* UCI style - e7e8q */
#define M_PR_EVAL      0x20                       /* separate captures */
#define M_PR_SEPARATE  0x40                       /* separate captures */
#define M_PR_LONG      0x80

//int move_print(int movenum, move_t *move, move_flags_t flags);
char *move_to_str(char *dst, const move_t move, __unused const int flags);
move_t move_from_str(const pos_t *pos, const char *str);
void moves_print(movelist_t *moves, int flags);
void move_sort_by_sq(movelist_t *moves);

#endif  /* MOVE_H */
