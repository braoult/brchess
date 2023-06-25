/* eval.h - static position evaluation.
 *
 * Copyright (C) 2021 Bruno Raoult ("br")
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

#include "position.h"

eval_t eval_material(pos_t *pos, bool color);
eval_t eval_mobility(pos_t *pos, bool color);
eval_t eval_square_control(pos_t *pos, bool color);

eval_t eval(pos_t *pos);

#endif  /* EVAL_H */
