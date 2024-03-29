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
#include "pool.h"
#include "piece.h"

/* move flags
 */
typedef unsigned char move_flags_t;
#define M_NORMAL       0x00
#define M_CHECK        0x01                       /* unsure if we know */
#define M_CAPTURE      0x02
#define M_EN_PASSANT   0x04
#define M_PROMOTION    0x08
#define M_CASTLE_K     0x10
#define M_CASTLE_Q     0x20

/* moves_print flags
 */
#define M_PR_CAPT      0x01
#define M_PR_NCAPT     0x02
#define M_PR_NUM       0x04
#define M_PR_NL        0x08
#define M_PR_EVAL      0x20                       /* separate captures */
#define M_PR_SEPARATE  0x40                       /* separate captures */
#define M_PR_LONG      0x80

typedef struct move_s {
    piece_t piece;
    square_t from, to;
    piece_t capture;                              /* captured piece */
    piece_t promotion;                            /* promoted piece */
    move_flags_t flags;
    eval_t negamax;
    eval_t eval;
    eval_t eval_simple;
    pos_t *pos;
    struct list_head list;                        /* next move */
} move_t;

pool_t *moves_pool_init();
void moves_pool_stats();
int move_print(int movenum, move_t *move, move_flags_t flags);
void moves_print(pos_t *move, move_flags_t flags);

void move_del(struct list_head *ptr);
int moves_del(pos_t *pos);

int pseudo_moves_castle(pos_t *pos, bool color, bool doit, bool do_king);
int pseudo_moves_gen(pos_t *pos, piece_list_t *piece, bool doit, bool do_king);
int pseudo_moves_pawn(pos_t *pos, piece_list_t *piece, bool doit);
int moves_gen(pos_t *pos, bool color, bool doit, bool do_king);
int moves_gen_king_moves(pos_t *pos, bool color, bool doit);

void moves_sort(pos_t *pos);
void moves_gen_eval_sort(pos_t *pos);

void moves_gen_all(pos_t *pos);
void moves_gen_all_nomoves(pos_t *pos);

pos_t *move_do(pos_t *pos, move_t *move);
void move_undo(pos_t *pos, move_t *move);

#endif  /* MOVE_H */
