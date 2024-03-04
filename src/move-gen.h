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

/**
 * gen_all_pseudomoves() - generate all pseudo moves
 *
 */
int gen_all_pseudomoves(pos_t *pos);

#endif  /* MOVEGEN_H */
