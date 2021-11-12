/* move.h - move management.
 *
 * Copyright (C) 2021 Bruno Raoult ("br")
 * Licensed under the GNU General Public License v3.0 or later.
 * Some rights reserved. See COPYING.
 *
 * You should have received a copy of the GNU General Public License along with this
 * program. If not, see <https://www.gnu.org/licenses/gpl-3.0-standalone.htmlL>.
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

/* move_add() return value when generating
 */


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
#define M_PR_SEPARATE  0x40                       /* separate capture/non capture */
#define M_PR_LONG      0x80
#define M_PR_CAPT      0x01
#define M_PR_NCAPT     0x02

typedef struct move_s {
    piece_t piece;
    square_t from, to;
    piece_t taken;                                /* removed piece */
    piece_t promotion;                            /* promoted piece */
    move_flags_t flags;
    struct list_head list;
} move_t;

pool_t *moves_pool_init();
int move_print(move_t *move, move_flags_t flags);
void moves_print(pos_t *move, move_flags_t flags);
int pseudo_moves_castle(pos_t *pos);

int pseudo_moves_gen(pos_t *pos, piece_list_t *piece, bool doit);
int pseudo_moves_pawn(pos_t *pos, piece_list_t *piece, bool doit);
int moves_gen(pos_t *pos, bool color, bool doit);

#endif  /* MODE_H */
