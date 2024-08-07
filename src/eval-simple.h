/* eval-simple.h - simple position evaluation.
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

#ifndef EVAL_SIMPLE_H
#define EVAL_SIMPLE_H

#include "chessdefs.h"
#include "piece.h"
#include "eval.h"

/* no queen on board */
#define simple_no_queen(p, c)              \
    ( !(p)->bb[c][QUEEN] )

#define simple_one_queen(p, c)             \
    ( popcount64((p)->bb[c][QUEEN]) == 1 )

#define simple_no_rook(p, c)               \
    (!(p)->bb[c][ROOK])

#define simple_one_minor_piece(p, c)       \
    (popcount64((p)->bb[c][KNIGHT] | (p)->bb[c][BISHOP]) == 1)

#define simple_is_endgame(p)                  \
    ( (simple_no_queen(p, WHITE)   ||         \
       (simple_one_queen(p, WHITE) &&         \
        simple_no_rook(p, WHITE)   &&         \
        simple_one_minor_piece(p, WHITE)))    \
      &&                                      \
      (simple_no_queen(p, BLACK)   ||         \
       (simple_one_queen(p, BLACK) &&         \
        simple_no_rook(p, BLACK)   &&         \
        simple_one_minor_piece(p, BLACK))) )

//s16 calc_phase(pos_t *pos);
//int eval_simple_find(char *str);
//void eval_simple_set(int set);
//void eval_simple_init(char *set);

eval_t eval_material(pos_t *pos);
eval_t eval_simple(pos_t *pos);

#endif  /* EVAL_SIMPLE_H */
