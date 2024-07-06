/* init.c - initialize all.
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
#include <unistd.h>
#include <locale.h>

#include "chessdefs.h"

#include "bitboard.h"
#include "hq.h"

#include "hash.h"
#include "hist.h"

#define printff(x) ({ printf(x); fflush(stdout); })
void init_all(void)
{
    /* line-buffered stdout */
    printff("initiazing stdout buffering... ");
    setlinebuf(stdout);

    /* for printf() numeric thousands separator */
    printff("locale... ");
    setlocale(LC_NUMERIC, "");

    /* pseudo random generator seed */
    printff("random generator... ");
    rand_init(RAND_SEED_DEFAULT);

    /* bitboards & hq */
    printff("bitboards... ");
    bitboard_init();

    printff("hq bitboards... ");
    hq_init();

    /* zobrist tables & default tt hashtable */
    printff("zobrist tables... ");
    zobrist_init();

    printff("transposition tables... ");
    tt_create(HASH_SIZE_DEFAULT);

    printf("done.\n");
}
