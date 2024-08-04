/* perft.h.h - perft.
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

#ifndef PERFT_H
#define PERFT_H

#include "position.h"

u64 perft(pos_t *pos, int depth,/* int ply,*/ bool divide);
u64 perft_alt(pos_t *pos, int depth, int ply, bool output);

#endif  /* PERFT_H */
