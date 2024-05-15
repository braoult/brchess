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
#include <locale.h>

#include "chessdefs.h"

#include "bitboard.h"
#include "hyperbola-quintessence.h"

#include "hash.h"

void init_all(void)
{
    /* for printf() numeric thousands separator */
    setlocale(LC_NUMERIC, "");
    /* line-buffered stdout */
    setlinebuf(stdout);

    /* pseudo random generator seed */
    rand_init(RAND_SEED_DEFAULT);

    /* bitboards & hq */
    bitboard_init();
    hyperbola_init();

    /* zobrist tables & default tt hashtable */
    zobrist_init();
    hash_create(HASH_SIZE_DEFAULT);

}
