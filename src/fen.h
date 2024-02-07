/* fen.h - fen notation.
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

#ifndef FEN_H
#define FEN_H

#include "position.h"

#define FENSTRLEN 92                              /* secure FEN string size */

extern const char *startfen;                      /* startup position */

extern position *startpos(position *pos);
extern position *fen2pos(position *pos, const char *fen);
extern char *pos2fen(const position *pos, char *fen);

#endif /* FEN_H */
