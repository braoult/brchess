/* util.c - various util functions.
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
#include <stdlib.h>

#include "util.h"
#include "bitboard.h"

/**
 * bitboard_print_simple - print simple bitboard representation
 * @bitboard: the bitboard
 * @title: a string or NULL
 */
void raw_bitboard_print(const bitboard bitboard, const char *title)
{
    if (title)
        printf("%s\n", title);
    for (rank r = RANK_8; r >= RANK_1; --r) {
        printf("%d ", r);
        for (file f = FILE_A; f <= FILE_H; ++f)
            printf(" %c", bitboard & (BB(f, r)) ? 'X': '.');
        printf("  A B C D E F G H\n");
    }
    printf(" \n");
    return;
}

/*int main()
{
    char *foo = safe_malloc(1000);
    exit(0);
}
*/
