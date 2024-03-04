/* fen-test.c - basic fen tests.
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

#include "chessdefs.h"
#include "bitboard.h"
#include "position.h"
#include "fen.h"

#include "common-test.h"

int main(__unused int ac, __unused char**av)
{
    pos_t *pos;
    const char *fen;
    char revfen[128];

    bitboard_init();

    while ((fen = next_fen(FEN))) {
        if (!(pos = fen2pos(NULL, fen))) {
            printf("fen = [%s] **INVALID\n", fen);
        } else {
            pos_print_raw(pos, 1);
            pos2fen(pos, revfen);
            printf("fen = [%s]\nrev = [%s]", fen, revfen);
            if (strcmp(fen, revfen))
                printf("  **FIXED\n");
            pos_del(pos);
        }
    }
}
