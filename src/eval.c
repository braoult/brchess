/* eval.c - static position evaluation.
 *
 * Copyright (C) 2021 Bruno Raoult ("br")
 * Licensed under the GNU General Public License v3.0 or later.
 * Some rights reserved. See COPYING.
 *
 * You should have received a copy of the GNU General Public License along with this
 * program. If not, see <https://www.gnu.org/licenses/gpl-3.0-standalone.htmlL>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later <https://spdx.org/licenses/GPL-3.0-or-later.html>
 *
 */

#include <stdio.h>

#include "eval.h"
#include "list.h"
#include "debug.h"

eval_t eval(pos_t *pos)
{
    eval_t eval[2] = {0}, res = 0;
    struct list_head *p_cur, *p_tmp, *list;
    piece_list_t *piece;

    /* 1) pieces value
     */
    for (int color=WHITE; color <=BLACK; ++color) {
        list = &pos->pieces[color];
        list_for_each_safe(p_cur, p_tmp, list) {
            piece = list_entry(p_cur, piece_list_t, list);
            eval[color] += piece->value;
        }
    }

    res += eval[WHITE] - eval[BLACK];
#   ifdef DEBUG_EVAL
    log_f(2, "pieces: W:%lu B:%lu diff=%lu eval=\n", eval[WHITE], eval[BLACK],
          res);
#   endif

    /* 2) square control
     */
    eval[WHITE] = popcount64(pos->controlled[WHITE]);
    eval[BLACK] = popcount64(pos->controlled[BLACK]);
    res += eval[WHITE] - eval[BLACK];
#   ifdef DEBUG_EVAL
    log_f(2, "square control: W:%lu B:%lu eval=%lu\n", eval[WHITE], eval[BLACK],
          res);
#   endif

    /* 3) mobility
     */


    return res;
}

#ifdef BIN_eval
#include "fen.h"
#include "move.h"

int main(int ac, char**av)
{
    pos_t *pos;

    debug_init(2);
    piece_pool_init();
    moves_pool_init();
    pos = pos_create();

    if (ac == 1) {
        pos_startpos(pos);
    } else {
        fen2pos(pos, av[1]);
    }

    moves_gen(pos, WHITE);
    moves_gen(pos, BLACK);
    pos_print(pos);
    pos_pieces_print(pos);

    printf("eval=%lu\n", eval(pos));
}
#endif
