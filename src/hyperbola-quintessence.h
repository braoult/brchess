/* hyperbola-quintessence.h - hyperbola-quintessence definitions.
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

#ifndef _HYPERBOLA_QUINTESSENCE_H
#define _HYPERBOLA_QUINTESSENCE_H

#include "board.h"
#include "bitboard.h"

void hyperbola_init(void);
extern bitboard_t hyperbola_bishop_moves(bitboard_t occ, square_t sq);
extern bitboard_t hyperbola_rook_moves(bitboard_t occ, square_t sq);
extern bitboard_t hyperbola_queen_moves(bitboard_t occ, square_t sq);

#endif  /* _HYPERBOLA_QUINTESSENCE_H */