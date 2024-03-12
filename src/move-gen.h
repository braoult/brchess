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
#include "hyperbola-quintessence.h"
#include "piece.h"
#include "move.h"

extern bool pseudo_is_legal(pos_t *pos, move_t move);
extern int pos_gen_pseudomoves(pos_t *pos);
extern movelist_t *pos_legalmoves(pos_t *pos, movelist_t *dest);

#endif  /* MOVEGEN_H */
