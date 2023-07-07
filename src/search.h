/* search.h - search for perfect move.
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

#ifndef SEARCH_H
#define SEARCH_H

#include <limits.h>
#include "position.h"

#define EVAL_MIN INT_MIN
#define EVAL_MAX INT_MAX

eval_t negamax(pos_t *pos, int depth);

#endif  /* SEARCH_H */
