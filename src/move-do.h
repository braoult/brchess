/* move-do.h - move do/undo.
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

#ifndef MOVE_DO_H
#define MOVE_DO_H

#include "position.h"

pos_t *move_do(pos_t *pos, const move_t move, state_t *state);
pos_t *move_undo(pos_t *pos, const move_t move, const state_t *state);

/* new version testing */
pos_t *move_do_alt(pos_t *pos, const move_t move, state_t *state);
pos_t *move_undo_alt(pos_t *pos, const move_t move, const state_t *state);

#endif  /* MOVE_DO_H */
