/* board.c - 8x8 board.
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

#include <ctype.h>

#include "brlib.h"
#include "board.h"

static const char *sq_strings[] = {
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
};

/**
 * sq_to_string() - return a square string
 * @square: square (0-64)
 *
 * @Return: Pointer to @square string representation ("a1"-"h8").
 */
const char *sq_to_string(const square_t square)
{
    return sq_strings[square];
}

/**
 * sq_from_string() - return a square from a string
 * @sqstr: the square representation (a1-h8)
 *
 * @Return: square_t representation of str.
 */
square_t sq_from_string(const char *sqstr)
{
    file_t f = C2FILE(sqstr[0]);
    rank_t r = C2RANK(sqstr[1]);
    return sq_coord_ok(f) && sq_coord_ok(r) ? sq_make(f, r): SQUARE_NONE;
}
