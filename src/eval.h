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
#include "piece.h"

/* max pieces eval is KING_VALUE + 9*QUEEN_VALUE + 2*ROOK_VALUE + 2*BISHOP_VALUE
 *  + 2*KNIGHT_VALUE which around 30000.
 * We are on secure side with -50000/+50000
 */
#define EVAL_MAX     (50000)
#define EVAL_MIN     (-EVAL_MAX)
#define EVAL_INVALID INT_MIN
#define EVAL_MATE    EVAL_MAX

eval_t eval_material(pos_t *pos, bool color);
eval_t eval_mobility(pos_t *pos, bool color);
eval_t eval_square_control(pos_t *pos, bool color);

eval_t eval(pos_t *pos);

#endif  /* EVAL_H */
