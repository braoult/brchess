/* position.h - position management definitions.
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

#ifndef POSITION_H
#define POSITION_H

#include <stdint.h>
#include "chessdefs.h"
#include "board.h"
#include "list.h"

typedef struct position {
    piece_t turn;                                 /* we use only color bit */
    castle_t castle;
    square_t en_passant;
    short clock_50;
    short curmove;
    eval_t eval;
    struct list_head pieces_white;
    struct list_head pieces_black;
    board_t *board;
} pos_t;

/* TODO: replace piece2string/piece2char with static array*/
char *piece2string(piece_t piece);
void pos_print(pos_t *pos);
pos_t *pos_init(pos_t *pos);
pos_t *pos_startpos(pos_t *pos);
pos_t *pos_create();

#endif
