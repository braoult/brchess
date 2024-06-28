/* tt-test.c - transposition table test.
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
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <locale.h>
#include <limits.h>

#include <brlib.h>

#include "chessdefs.h"
#include "fen.h"
#include "position.h"
#include "move.h"
#include "move-do.h"
#include "move-gen.h"
#include "search.h"

static void pr_entry(hentry_t *entry)
{
    if (!entry)
        printf("entry: NULL\n");
    else {
        printf("entry: key=%lx depth=%d n=%lu\n",
               entry->key, HASH_PERFT_DEPTH(entry->data),
               HASH_PERFT_VAL(entry->data));
    }
}

int main()
{
    pos_t *pos = NULL;
    char *token, *str, buf[128];
    hentry_t *entry;
    move_t move;
    state_t state;
    movelist_t movelist;

    const char *moves_array[] = {
        "e2e4 e7e5 g1f3 b8c6",
        "e2e4 b8c6 g1f3 e7e5"
    };

    init_all();

    for (uint i = 0; i < ARRAY_SIZE(moves_array); ++i) {
        int depth = 0;
        str = strdup(moves_array[i]);
        printf("%2d: ", i + 1);

        pos = startpos(pos);
        entry = tt_store_perft(pos->key, 0, 123 + depth);
        pr_entry(entry);
        token = strtok(str, " \t");
        while (token) {
            depth++;
            printf("%s ", token);

            move = move_from_str(token);
            pos_set_checkers_pinners_blockers(pos);
            pos_legal(pos, pos_gen_pseudo(pos, &movelist));
            printf("move: %s\n", move_to_str(buf, move, 0));
            move =  move_find_in_movelist(move, &movelist);
            if (move != MOVE_NONE) {
                move_do(pos, move, &state);
                if ((entry = tt_probe_perft(pos->key, depth))) {
                    printf("tt hit: depth=%d val=%lu",
                           HASH_PERFT_DEPTH(entry->data),
                           HASH_PERFT_VAL(entry->data));
                } else {
                    tt_store_perft(pos->key, i + 1, depth);
                    printf("tt store: depth=%d val=%lu", depth, (u64)i * 123);
                };
            }

            token = strtok(NULL, " \t");
        }
        printf("\n");
        free(str);
    }
    return 0;
}

/* ccls bug report: https://github.com/emacs-lsp/emacs-ccls/issues/126
 */
/*
 * int called(int), caller();
 *
 * /\**
 *  * called() - test ccls.
 *  * @x: int, the test value
 *  *
 *  * @called() description.
 *  *
 *  * @return: int, a very interesting value.
 *  *\/
 * int called(int x) { return x; }
 *
 * int caller()
 * {
 *     int i = 0;
 *     called(int x)
 *     return i;
 * }
 */
