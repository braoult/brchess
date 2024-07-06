/* attack-test.c - basic square attack tests.
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
#include "move-gen.h"
#include "attack.h"

#include "common-test.h"

int main(int __unused ac, __unused char**av)
{
    int i = 0;
    char *fen;
    pos_t *pos;//, *fishpos = pos_new();
    bitboard_t checkers, pinners, blockers;

    setlinebuf(stdout);                           /* line-buffered stdout */

    bitboard_init();
    hq_init();

    while ((fen = next_fen(ATTACK))) {
        //printf(">>>>> %s\n", test[i]);
        printf("original fen %d: [%p][%s]\n", i, fen, fen);
        if (!(pos = fen2pos(NULL, fen))) {
            printf("wrong fen %d: [%s]\n", i, fen);
            continue;
        }
        pos->checkers = pos_checkers(pos, pos->turn);
        pos_set_pinners_blockers(pos);

        pos_print(pos);

        checkers = pos->checkers;
        pinners = pos->pinners;
        blockers = pos->blockers;

        pos_set_checkers_pinners_blockers(pos);

        printf("******* line %d: %s\n", cur_line(), cur_comment());
        bb_print_multi("checkers", 2, checkers, pos->checkers);
        bb_print_multi("pinners", 2, pinners, pos->pinners);
        bb_print_multi("blockers", 2, blockers, pos->blockers);

        pos_del(pos);
        i++;
    }
    return 0;
}
