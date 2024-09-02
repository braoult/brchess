/* pv.c - Principal variation.
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

#include <stdio.h>
#include <ctype.h>

#include <brlib.h>

#include "move.h"

/*
 * For PV, we use a triangular structure. The following table is adapted from
 * https://www.chessprogramming.org/Triangular_PV-Table.
 *
 * ply     Search   maxLengthPV
 *         Depth
 *                  +--------------------------------------------+
 * 0=root  N        |N                                           |
 *                  +------------------------------------------+-+
 * 1       N-1      |N-1                                       |
 *                  +----------------------------------------+-+
 * 2       N-2      |N-2                                     |
 *                  +----------------------------------------+-+
 * ...                                     /
 *                  +-----+-+
 * N-3     3        |3    |
 *                  +---+-+
 * N-2     2        |2  |
 *                  +-+-+
 * N-1     1        |1|
 *                  +-+
 *
 * The offsets will be:
 * Depth (=size):  1  2  3  4  5  6  7 ...
 * Offset:         0  1  3  6 10 15 21 ...
 *
 * Off(n)     = n * (n - 1) / 2      (direct calc)
 * Off(n + 1) = Off(n) + n           (calc from previous)
 */

typedef struct {
    move[];
} foo;
