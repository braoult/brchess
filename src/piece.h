/* piece.h - piece definitions.
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

#ifndef PIECE_H
#define PIECE_H

#include <ctype.h>

#include "chessdefs.h"
#include "list.h"
#include "pool.h"


/* default values for opening, midgame, endgame
 */
#define E_VAL_OPN    0                            /* empty */
#define P_VAL_OPN    100
#define N_VAL_OPN    300
#define B_VAL_OPN    300
#define R_VAL_OPN    500
#define Q_VAL_OPN    900
#define K_VAL_OPN    20000

#define E_VAL_MID    0
#define P_VAL_MID    100
#define N_VAL_MID    300
#define B_VAL_MID    300
#define R_VAL_MID    500
#define Q_VAL_MID    900
#define K_VAL_MID    20000

#define E_VAL_END    0
#define P_VAL_END    100
#define N_VAL_END    300
#define B_VAL_END    300
#define R_VAL_END    500
#define Q_VAL_END    900
#define K_VAL_END    20000

/*
typedef struct piece_list_s {
    piece_t piece;
    square_t square;
    short castle;
    s64 value;
    struct list_head list;
} piece_list_t;
*/

/* some default values for pieces
 */
extern const struct piece_details {
    char abbrev;                                /* used for game notation */
    //char abbrev_b;
    char *symbol;
    //char *symbol_b;                               /* used for game notation */
    char *name;
    s64  opn_value;
    s64  mid_value;
    s64  end_value;
} piece_details[];

#define P_ABBR(p)      piece_details[PIECE(p)].abbrev
#define P_SYM(p)       piece_details[PIECE(p)].symbol
#define P_NAME(p)      piece_details[PIECE(p)].name
#define P_VAL(p)       piece_details[PIECE(p)].opn_value

/* use short name or symbol - no effect
 */
#define P_USE_UTF      1

//void piece_list_print(struct list_head *list);
//pool_t *piece_pool_init();
//void piece_pool_stats();
//piece_list_t *piece_add(pos_t *pos, piece_t piece, square_t square);
//void piece_del(struct list_head *ptr);
//int pieces_del(pos_t *pos, short color);

#endif
