/* move-gen.h - move generation
 *
 * Copyright (C) 2024 Bruno Raoult ("br")
 * Licensed under the GNU General Public License v3.0 or later.
 * Some rights reserved. See COPYING.
 *
 * You should have received a copy of the GNU General Public License along with this
 * program. If not, see <https://www.gnu.org/licenses/gpl-3.0-standalone.html>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later <https://spdx.org/licenses/GPL-3.0-or-later.html>
 *
 */

#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "bitops.h"

#include "bitboard.h"
#include "hq.h"
#include "piece.h"
#include "move.h"

bool pseudo_is_legal(const pos_t *pos, const move_t move);
move_t pos_next_legal(const pos_t *pos, movelist_t *movelist, int *start);
movelist_t *pos_legal_dup(const pos_t *pos, movelist_t *pseudo, movelist_t *legal);
movelist_t *pos_legal(const pos_t *pos, movelist_t *list);

movelist_t *pos_gen_pseudo(pos_t *pos, movelist_t *movelist);

#endif  /* MOVEGEN_H */
