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

//eval_t negamax(pos_t *pos, int depth, int color);
//eval_t pvs(pos_t *pos, int depth, int alpha, int beta, int color);

u64 perft(pos_t *pos, int depth, int ply);
u64 perft2(pos_t *pos, int depth, int ply);
u64 perft_new_pinners(pos_t *pos, int depth, int ply);

#endif  /* SEARCH_H */
