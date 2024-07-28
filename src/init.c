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
#include "util.h"
#include "bitboard.h"
#include "hq.h"
#include "eval-defs.h"
#include "hash.h"
#include "hist.h"

#define printff(x) ({ printf(x); fflush(stdout); })

void init_all(void)
{
    printff("initiazing system: ");
    /* line-buffered stdout */
    printff("stdout buffering... ");
    setlinebuf(stdout);

    printff("locale... ");
    /* for printf() thousands separator */
    setlocale(LC_NUMERIC, "");
    setlocale(LC_CTYPE, "C");

    /* pseudo random generator seed */
    printff("random generator... ");
    rand_init(RAND_SEED_DEFAULT);

    printf("done.\n");

    printff("initiazing board data: ");
    /* bitboards & hq */
    printff("bitboards... ");
    bitboard_init();

    printff("hq bitboards... ");
    hq_init();

    printf("done.\n");

    printff("initiazing move data: ");
    /* zobrist tables & default tt hashtable */
    printff("zobrist tables... ");
    zobrist_init();

    printff("transposition tables... ");
    tt_create(HASH_SIZE_DEFAULT);

    printf("done.\n");

    printff("initiazing eval data: ");
    printf("parameters... ");
    param_init();

    /* eval tables */
    printf("pst tables... ");
    pst_init(PST_DEFAULT);

    printf("done.\n");
}
