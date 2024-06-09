/* movedo-test.c - basic movedo/undo tests.
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
#include <unistd.h>

#include "chessdefs.h"
#include "fen.h"
#include "position.h"
#include "move.h"
#include "move-do.h"
#include "move-gen.h"

#include "common-test.h"

int main(int __unused ac, __unused char**av)
{
    int i = 0, test_line;
    char *fen, movebuf[8];;
    pos_t *pos, *savepos;
    movelist_t movelist;
    move_t *move, *last;

    init_all();

    while ((fen = next_fen(MOVEDO))) {
        test_line = cur_line();
        if (!(pos = fen2pos(NULL, fen))) {
            printf("wrong fen %d: [%s]\n", i, fen);
            continue;
        }

        movelist.nmoves = 0;
        pos_set_checkers_pinners_blockers(pos);
        pos_legal(pos, pos_gen_pseudo(pos, &movelist));
        last = movelist.move + movelist.nmoves;
        savepos = pos_dup(pos);

        state_t state = pos->state;
        int j = 0;
        for (move = movelist.move; move < last; ++move) {
            //pos_print(pos);
            //printf("i=%d j=%d  turn=%d move=[%s]\n", i, j, pos->turn,
            //       move_str(movebuf, move, 0));
            //move_p
            move_do(pos, *move, &state);
            //pos_print(pos);
            //fflush(stdout);
            if (!pos_ok(pos, false)) {
                printf("*** fen %d [%s] move %d [%s] invalid position after move_do\n",
                       test_line, fen, j, move_str(movebuf, *move, 0));
                exit(0);
            }

            //printf("%d/%d move_do check ok\n", i, j);
            move_undo(pos, *move, &state);
            pos->state = state;
            if (!pos_ok(pos, false)) {
                printf("*** fen %d [%s] move %d [%s] invalid position after move_undo\n",
                       test_line, fen, j, movebuf);
                exit(0);
            }
            if (pos_cmp(pos, savepos) != true) {
                printf("*** fen %d [%s] move %d [%s] position differ after move_{do,undo}\n",
                       test_line, fen, j, movebuf);
                exit(0);
            }
            //fflush(stdout);
            //pos_check(pos, true);
            //printf("%d/%d move_undo check ok\n", i, j);
            j++;
        }
        printf("fen %d line=%d [%s] %d move_{do,undo} OK\n", i, test_line, fen, j);
        pos_del(savepos);
        pos_del(pos);
        i++;
    }
    return 0;
}
