/* eval.c - static position evaluation.
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

#include <stdio.h>

#include <brlib.h>

#include "chessdefs.h"
#include "position.h"
#include "eval.h"
#include "eval-simple.h"

/*
 * inline eval_t eval_mobility(pos_t *pos, bool color)
 * {
 *     return pos->mobility[color];
 * }
 *
 *
 * inline eval_t eval_square_control(pos_t *pos, bool color)
 * {
 *     return popcount64(pos->controlled[color]);
 * }
 */

eval_t eval(pos_t *pos)
{
    //eval_t simple = 0, control[2] = {0};
    eval_t simple = 0;

    //if (pos->eval != EVAL_INVALID)
    //    return pos->eval;

    /* 1) pieces value */
    //material[WHITE] = eval_material(pos, WHITE);
    //material[BLACK] = eval_material(pos, BLACK);
    simple = eval_simple(pos);

#   ifdef DEBUG_EVAL2
    log_f(2, "eval_simple=%d\n", simple);
#   endif

    /* 2) square control: 10 square controls diff = 1 pawn */
    //control[WHITE] = eval_square_control(pos, WHITE);
    //control[BLACK] = eval_square_control(pos, BLACK);

#   ifdef DEBUG_EVAL2
    log_f(2, "square control: W:%d B:%d diff=%d\n",
          control[WHITE], control[BLACK],
          (control[WHITE] - control[BLACK]) * 10);
#   endif

    /* 3) mobility: 10 mobility diff = 1 pawn
     */
#   ifdef DEBUG_EVAL2
    log_f(2, "mobility: W:%u B:%u diff=%d\n",
          pos->mobility[WHITE], pos->mobility[BLACK],
          (pos->mobility[WHITE] - pos->mobility[BLACK]) * 10);
#   endif

    //eval_t res = simple +
    //    (control[WHITE] - control[BLACK]) * 10 +
    //    (pos->mobility[WHITE] - pos->mobility[BLACK]) * 10;
#   ifdef DEBUG_EVAL2
    log_f(2, "eval: %d\n", res);
#   endif
    pos->eval = simple;
    return simple;
}
