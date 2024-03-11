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
 * 3      2 2    1 1 1 1 1 1
 * 1      5 3    8 7 5 4 2 1    6 5    0
 * UUUUUUUU FFFFFF ppp ccc tttttt ffffff
 *
 * bits    len  range           type       mask        get   desc
 * ffffff    6    0-5       square_t         3f        &63   from
 * tttttt    6   6-11       square_t        fc0   (>>6)&63   to
 * ccc       3  12-14   piece_type_t       7000   (>>12)&7   captured
 * ppp       3  15-17   piece_type_t      38000   (>>15)&7   promoted
 * FFFFFF    6  18-23   move_flags_t     fc0000        N/A   flags
 * UUUUUUUU  8  24-31         unused   fe000000        N/A   future usage ?
 */
typedef u32 move_t;

typedef enum {
    M_START     = 18,
    M_CAPTURE   = (1 << (M_START + 0)),
    M_ENPASSANT = (1 << (M_START + 1)),
    M_PROMOTION = (1 << (M_START + 2)),
    M_CASTLE_K  = (1 << (M_START + 3)),           /* maybe only one ? */
    M_CASTLE_Q  = (1 << (M_START + 4)),           /* maybe only one ? */
    M_CHECK     = (1 << (M_START + 5))            /* maybe unknown/useless ? */
} move_type_t;

#define MOVES_MAX   256

typedef struct __movelist_s {
    move_t move[MOVES_MAX];
    int nmoves;                                   /* total moves (fill) */
    int curmove;                                  /* current move (use) */
} movelist_t;


/* move flags */
//#define M_FLAGS_BEG 18
//#define M_HAS_FLAGS mask(M_FLAGS_BEG + 0)       /* probably useless */
//#define M_CAPTURE     mask(M_FLAGS_BEG + 0)
//#define M_EN_PASSANT  mask(M_FLAGS_BEG + 1)
//#define M_PROMOTION   mask(M_FLAGS_BEG + 2)
//#define M_CASTLE_K    mask(M_FLAGS_BEG + 3)       /* maybe only one ? */
//#define M_CASTLE_Q    mask(M_FLAGS_BEG + 4)       /* maybe only one ? */
//#define M_CHECK       mask(M_FLAGS_BEG + 5)       /* probably unknown/useless */

//#define M_FLAGS     (M_CAPTURE | M_ENPASSANT | M_PROMOTION |
//                     M_CASTLE_K | M_CASTLE_Q | M_CHECK)
//#define M_NORMAL    (~M_FLAGS)

static inline square_t move_from(move_t move)
{
    return move & 077;
}
static inline square_t move_to(move_t move)
{
    return (move >> 6) & 077;
}

static inline piece_t move_promoted(move_t move)
{
    return (move >> 15) & 07;
}

static inline piece_t move_captured(move_t move)
{
    return (move >> 12) & 07;
}

static inline move_t move_make(square_t from, square_t to)
{
    return (to << 6) | from;
}

static inline move_t move_make_flags(square_t from, square_t to, move_type_t flags)
{
    return move_make(from, to) | flags;
}

static inline move_t move_make_capture(square_t from, square_t to)
{
    return move_make_flags(from, to, M_CAPTURE);
}

static inline move_t move_make_promote(square_t from, square_t to,
                                       piece_type_t promoted)
{
    return move_make_flags(from, to, M_PROMOTION) | (promoted << 15);
}

static inline move_t move_make_promote_capture(square_t from, square_t to,
                                               piece_type_t promoted)
{
    return move_make_flags(from, to, M_CAPTURE | M_PROMOTION) | (promoted << 15);
}

/* moves_print flags
 */
#define M_PR_CAPT      0x01
#define M_PR_NCAPT     0x02
#define M_PR_NUM       0x04
#define M_PR_NL        0x08
#define M_PR_EVAL      0x20                       /* separate captures */
#define M_PR_SEPARATE  0x40                       /* separate captures */
#define M_PR_LONG      0x80

//pool_t *moves_pool_init();
//void moves_pool_stats();

//int move_print(int movenum, move_t *move, move_flags_t flags);

extern void moves_print(pos_t *pos, int flags);
extern void move_sort_by_sq(pos_t *pos);

//extern int pseudo_moves_castle(pos_t *pos, bool color, bool doit, bool do_king);
//int pseudo_moves_gen(pos_t *pos, piece_list_t *piece, bool doit, bool do_king);
//int pseudo_moves_pawn(pos_t *pos, piece_list_t *piece, bool doit);
//extern int moves_gen(pos_t *pos, bool color, bool doit, bool do_king);
//extern int moves_gen_king_moves(pos_t *pos, bool color, bool doit);

// extern void moves_sort(pos_t *pos);

//extern void moves_gen_eval_sort(pos_t *pos);

//extern void moves_gen_all(pos_t *pos);
//extern void moves_gen_all_nomoves(pos_t *pos);

//extern pos_t *move_do(pos_t *pos, move_t *move);
//extern void move_undo(pos_t *pos, move_t *move);

#endif  /* MOVE_H */
