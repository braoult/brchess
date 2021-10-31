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

#ifndef ROOK_H
#define ROOK_H

#include "chessdefs.h"
#include "position.h"
#include "pool.h"
#include "piece.h"

typedef struct move_s {
    piece_t piece;
    square_t from, to;
    piece_t taken;                                /* removed piece */

    struct list_head list;
} move_t;

pool_t *moves_pool_init();
void move_print(move_t *move);
void moves_print(pos_t *move);
int pseudo_moves_get(pos_t *pos, piece_list_t *piece);
int moves_get(pos_t *pos);

/* not used */
move_t *pseudo_moves_pawn(pos_t *pos);

#endif
