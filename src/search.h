/* search.h - search for perfect move.
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

#ifndef SEARCH_H
#define SEARCH_H

#include "position.h"

/**
 * search_values_t - search values given by "go" command
 * @depth: max depth "go depth X", MAX_DEPTH for no limit
 *
 *
 */
typedef struct {
    u16 depth;                                    /* go depth X */
} search_uci_t;

extern search_uci_t search_uci;

bool is_draw(pos_t *pos);
eval_t negamax(pos_t *pos, int depth, int color);
void search(pos_t *pos);
//eval_t pvs(pos_t *pos, int depth, int alpha, int beta, int color);

#endif  /* SEARCH_H */
