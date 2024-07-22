/* eval.h - static position evaluation.
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

#ifndef EVAL_H
#define EVAL_H

#include <limits.h>

#include "chessdefs.h"

/* game phases
 */
enum {
    MIDGAME,
    ENDGAME,
    PHASE_NB
};
typedef s16 phase_t;

/* max pieces eval is 9*QUEEN_VALUE + 2*ROOK_VALUE + 2*BISHOP_VALUE
 * + 2*KNIGHT_VALUE which is (for a pawn valued at 100) well less than 15,000.
 */
#define EVAL_MAX     (SHRT_MAX)                   /* 32767 */
#define EVAL_MIN     (-EVAL_MAX)

#define EVAL_INVALID EVAL_MIN

#define EVAL_MATE    30000

eval_t eval_mobility(pos_t *pos, bool color);
eval_t eval_square_control(pos_t *pos, bool color);

eval_t eval(pos_t *pos);

#endif  /* EVAL_H */
