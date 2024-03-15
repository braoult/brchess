/* attack.h - attack functions.
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

#ifndef _ATTACK_H
#define _ATTACK_H

#include "chessdefs.h"
#include "bitboard.h"

bitboard_t sq_attackers(const pos_t *pos, const bitboard_t occ, const square_t sq, const color_t c);
bitboard_t sq_attackers_all(const pos_t *pos, const square_t sq);
bitboard_t sq_pinners(const pos_t *pos, const square_t sq, const color_t c);
#endif
