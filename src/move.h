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
#include "position.h"
//#include "pool.h"
#include "piece.h"

/* move structure:
 * 3     2 2     1 1 1 1 1 1
 * 1     5 4     8 7 5 4 2 1    6 5    0
 * UUUUUUU FFFFFFF ppp ccc tttttt ffffff
 *
 * bits    range         type     mask        get   desc
 * ffffff    0-5     square_t       3f        &63   from
 * tttttt   6-11     square_t      fc0   (>>6)&63   to
 * ccc     12-14 piece_type_t     7000   (>>12)&7   captured
 * ppp     15-17 piece_type_t    38000   (>>15)&7   promoted
 * FFFFFFF 18-24          N/A  1fc0000        N/A   flags
 * UUUUUUU 25-31       unused fe000000        N/A   future usage ?
 */
#define move_t u32

/* move flags */
#define M_FLAGS_BEG 18
#define M_HAS_FLAGS mask(M_FLAGS_BEG + 0)         /* probably unused */
#define M_CAPTURE   mask(M_FLAGS_BEG + 1)
#define M_ENPASSANT mask(M_FLAGS_BEG + 2)
#define M_PROMOTION mask(M_FLAGS_BEG + 3)
#define M_CASTLE_K  mask(M_FLAGS_BEG + 4)
#define M_CASTLE_Q  mask(M_FLAGS_BEG + 5)
#define M_CHECK     mask(M_FLAGS_BEG + 6)         /* probably unknown/useless */

#define M_FLAGS     (M_CAPTURE | M_ENPASSANT | M_PROMOTION | \
                     M_CASTLE_K | M_CASTLE_Q | M_CHECK)
#define M_NORMAL    (~M_FLAGS)

#define MOVES_MAX   256

static inline move_t move_make(square_t from, square_t to)
{
    return (to << 3) | from;
}

static inline move_t move_make_promote(square_t from, square_t to, piece_type_t piece)
{
    return move_make(from, to) | M_ENPASSANT | (piece << 15);
}

static inline move_t move_from(move_t move)
{
    return move & 56;
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

typedef struct {
    move_t move[MOVES_MAX];
    int nmoves;                                   /* total moves (fill) */
    int curmove;                                  /* current move (use) */
} movelist_t;

//pool_t *moves_pool_init();
//void moves_pool_stats();
//int move_print(int movenum, move_t *move, move_flags_t flags);
//void moves_print(pos_t *move, move_flags_t flags);

//void move_del(struct list_head *ptr);
//int moves_del(pos_t *pos);

int pseudo_moves_castle(pos_t *pos, bool color, bool doit, bool do_king);
//int pseudo_moves_gen(pos_t *pos, piece_list_t *piece, bool doit, bool do_king);
//int pseudo_moves_pawn(pos_t *pos, piece_list_t *piece, bool doit);
//extern int moves_gen(pos_t *pos, bool color, bool doit, bool do_king);
//extern int moves_gen_king_moves(pos_t *pos, bool color, bool doit);

//extern void moves_sort(pos_t *pos);
//extern void moves_gen_eval_sort(pos_t *pos);

//extern void moves_gen_all(pos_t *pos);
//extern void moves_gen_all_nomoves(pos_t *pos);

//extern pos_t *move_do(pos_t *pos, move_t *move);
//extern void move_undo(pos_t *pos, move_t *move);

#endif  /* MOVE_H */
