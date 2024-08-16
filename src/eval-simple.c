/* eval-simple.c - simple position evaluation.
 *
 * Copyright (C) 2023 Bruno Raoult ("br")
 * Licensed under the GNU General Public License v3.0 or later.
 * Some rights reserved. See COPYING.
 *
 * You should have received a copy of the GNU General Public License along with this
 * program. If not, see <https://www.gnu.org/licenses/gpl-3.0-standalone.html>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later <https://spdx.org/licenses/GPL-3.0-or-later.html>
 *
 */

#include <brlib.h>
#include <bitops.h>

#include "chessdefs.h"
#include "piece.h"
#include "position.h"
#include "eval-simple.h"
#include "eval.h"

/**
 * eval_material() - eval position material
 * @pos: &position to evaluate
 *
 * Basic material evaluation. Only midgame value is used.
 *
 * @return: the @pos material evaluation in centipawns
 */
eval_t eval_material(pos_t *pos)
{
    eval_t val[COLOR_NB] = { 0 };

    for (piece_type_t pt = PAWN; pt < KING; ++pt) {
        eval_t pval = piece_midval(pt);
        val[WHITE] += popcount64(pos->bb[WHITE][pt]) * pval;
        val[BLACK] += popcount64(pos->bb[BLACK][pt]) * pval;
    }
#   ifdef DEBUG_EVAL
    printf("material: w:%d b:%d\n", val[WHITE], val[BLACK]);
#   endif

    return val[WHITE] - val[BLACK];
}

/**
 * eval_simple() - simple and fast position evaluation
 * @pos: &position to evaluate
 *
 * @return: the @pos evaluation in centipawns
 */
eval_t eval_simple(pos_t *pos)
{
    eval_t eval[2] = { 0, 0 }, res;
    eval_t mg_eval[2], eg_eval[2];

    int eg_weight = clamp((int) pos->phase, ALL_PHASE, 0);
    int mg_weight = ALL_PHASE - pos->phase;

    for (color_t color = WHITE; color < COLOR_NB; ++color) {
        mg_eval[color] = 0;
        eg_eval[color] = 0;
        for (piece_type_t pt = PAWN; pt < KING; pt++) {
            bitboard_t bb = pos->bb[color][pt];
            while (bb) {
                square_t sq = bb_next(&bb);
                mg_eval[color] += pst_mg[color][pt][sq];
                eg_eval[color] += pst_eg[color][pt][sq];
            }

#           ifdef DEBUG_EVAL
            printf("c=%d pt=%d mg=%d eg=%d\n", color, pt,
                   mg_eval[color], eg_eval[color]);
#           endif

        }
    }
    eval[WHITE] = mg_eval[WHITE] * mg_weight + eg_eval[WHITE] * eg_weight;
    eval[BLACK] = mg_eval[BLACK] * mg_weight + eg_eval[BLACK] * eg_weight;
    res = (eval[WHITE] - eval[BLACK]) / ALL_PHASE;
#   ifdef DEBUG_EVAL
    printf("phase:%d mg:%d/%d eg:%d/%d ev:%d/%d RES=%d\n",
           pos->phase, mg_eval[WHITE], mg_eval[BLACK],
           eg_eval[WHITE], eg_eval[BLACK],
           eval[WHITE], eval[BLACK], res
        );
#   endif
    return res;
}
