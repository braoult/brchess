/* piece.h - piece definitions.
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

#ifndef PIECE_H
#define PIECE_H

#include "chessdefs.h"
#include "board.h"
#include "list.h"
#include "position.h"
#include "pool.h"

#define PIECE_DEFAULT_VALUE 0

/* initial default values */
#define PAWN_VALUE    100
#define KNIGHT_VALUE  300
#define BISHOP_VALUE  300
#define ROOK_VALUE    500
#define QUEEN_VALUE   900
#define KING_VALUE    20000

typedef struct {
    piece_t piece;
    square_t square;
    short castle;
    int64_t value;
    struct list_head list;
} piece_list_t;

void piece_list_print(struct list_head *list);
void pieces_print_pos_pieces(pos_t *pos);
pool_t *piece_pool_init();
piece_list_t *piece_add(pos_t *pos, piece_t piece, square_t square);

#endif
