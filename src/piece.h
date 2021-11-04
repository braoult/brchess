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

#include <ctype.h>

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

typedef struct piece_list_s {
    piece_t piece;
    square_t square;
    short castle;
    int64_t value;
    struct list_head list;
} piece_list_t;

/* some default values for pieces
 */
extern struct piece_details {
    char abbrev_w;                                /* used for game notation */
    char abbrev_b;
    char *symbol_w;
    char *symbol_b;                               /* used for game notation */
    char *name;
    int64_t value;
} piece_details[];

#define P_NAME(p)      piece_details[PIECE(p)].name
#define P_LETTER(p)    piece_details[PIECE(p)].abbrev_w
#define P_SYM(p)       piece_details[PIECE(p)].symbol_b
#define P_CSHORT(p)    (COLOR(p) == WHITE? piece_details[PIECE(p)].abbrev_w: \
                        piece_details[PIECE(p)].abbrev_b)
#define P_CSYM(p)      (COLOR(p) == WHITE? piece_details[PIECE(p)].symbol_w: \
                        piece_details[PIECE(p)].symbol_b)
/* use short name or symbol - no effect
 */
#define P_USE_UTF      1

void piece_list_print(struct list_head *list);
void pieces_print_pos_pieces(pos_t *pos);
pool_t *piece_pool_init();
piece_list_t *piece_add(pos_t *pos, piece_t piece, square_t square);

#endif
