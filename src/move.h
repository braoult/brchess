/* move.h - move management.
 *
 * Copyright (C) 2021 Bruno Raoult ("br")
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
 * 3 3     2 2    1 1 1 1 1 1
 * 1 0     4 3    8 7 5 4 2 1    6 5    0
 * S UUUUUUU FFFFFF ccc ppp tttttt ffffff
 *
 * bits    len off range         type         mask        get  desc
 * ffffff    6   0   0-5     square_t          077        &077 from
 * tttttt    6   6  6-11     square_t        07700 (>>6)  &077 to
 * ppp       3  12 12-14 piece_type_t       070000 (>>12) &07  promoted
 * ccc       3  15 15-17 piece_type_t      0700000 (>>15) &07  captured
 * FFFFFF    6  18 18-23 move_flags_t    077000000 (>>18) &077 N/A  flags
 * UUUUUUU   7  24 24-30       unused 017700000000             future usage
 * S         1  31 31-31            - 020000000000             sign
 */
typedef s32 move_t;

/* special move_t values */
#define MOVE_NONE ((move_t) -1)
#define MOVE_NULL ((move_t) 0)                    /* hack: from = to = A1 */

enum {
    M_OFF_FROM     = 0,
    M_OFF_TO       = 6,
    M_OFF_PROMOTED = 12,
    M_OFF_CAPTURED = 15,
    M_OFF_FLAGS    = 18
};

typedef enum {
    M_CAPTURE   = mask(M_OFF_FLAGS + 0),
    M_ENPASSANT = mask(M_OFF_FLAGS + 1),
    M_PROMOTION = mask(M_OFF_FLAGS + 2),
    M_CASTLE_K  = mask(M_OFF_FLAGS + 3),          /* maybe only one ? */
    M_CASTLE_Q  = mask(M_OFF_FLAGS + 5),          /* maybe only one ? */
    M_CHECK     = mask(M_OFF_FLAGS + 6),          /* maybe unknown/useless ? */
    M_DPUSH     = mask(M_OFF_FLAGS + 7)           /* pawn double push */
} move_flags_t;

#define move_set_flags(move, flags) ((move) | (flags))

#define is_capture(m)   ((m) & M_CAPTURE)
#define is_enpassant(m) ((m) & M_ENPASSANT)
#define is_promotion(m) ((m) & M_PROMOTION)
#define is_castle(m)    ((m) & (M_CASTLE_K | M_CASTLE_Q))
#define is_castle_K(m)  ((m) & M_CASTLE_K)
#define is_castle_Q(m)  ((m) & M_CASTLE_Q)
#define is_check(m)     ((m) & M_CHECK)
#define is_dpush(m)     ((m) & M_DPUSH)

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

static inline piece_type_t move_captured(move_t move)
{
    return (move >> M_OFF_CAPTURED) & 07;
}

static inline move_t move_make(square_t from, square_t to)
{
    return (to << M_OFF_TO) | from;
}

static inline move_t move_make_flags(square_t from, square_t to, move_flags_t flags)
{
    return move_set_flags(move_make(from, to), flags);
}

static inline move_t move_make_capture(square_t from, square_t to)
{
    return move_make_flags(from, to, M_CAPTURE);
}

static inline move_t move_make_enpassant(square_t from, square_t to)
{
    return move_make_flags(from, to, M_ENPASSANT);
}

static inline move_t move_make_promote(square_t from, square_t to,
                                       piece_type_t promoted)
{
    return move_make_flags(from, to, M_PROMOTION) | (promoted << M_OFF_PROMOTED);
}

static inline move_t move_make_promote_capture(square_t from, square_t to,
                                               piece_type_t promoted)
{
    return move_make_promote(from, to, promoted) | M_CAPTURE;
}

static inline move_t move_set_captured(move_t move, piece_type_t captured)
{
    return move | (captured << M_OFF_CAPTURED);
}

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
char *move_str(char *dst, const move_t move, __unused const int flags);
void moves_print(movelist_t *moves, int flags);
void move_sort_by_sq(movelist_t *moves);

#endif  /* MOVE_H */
