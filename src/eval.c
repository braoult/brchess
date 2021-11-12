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
    eval_t material[2] = {0};
    eval_t control[2] = {0};
    eval_t res = 0;
    struct list_head *p_cur, *p_tmp, *list;
    piece_list_t *piece;

    /* 1) pieces value
     */
    for (int color=0; color <2; ++color) {
        list = &pos->pieces[color];
        list_for_each_safe(p_cur, p_tmp, list) {
            piece = list_entry(p_cur, piece_list_t, list);
            if (PIECE(piece->piece) != KING)
                material[color] += piece->value;
        }
    }

    res = material[WHITE] - material[BLACK];
#   ifdef DEBUG_EVAL
    log_f(2, "material: W:%ld B:%ld eval=%ld (%.3f pawns)\n",
          material[WHITE], material[BLACK],
          material[WHITE] - material[BLACK], (float)res/100);
#   endif

    /* 2) square control: 10 square controls diff = 1 pawn
     */
    control[WHITE] = popcount64(pos->controlled[WHITE]);
    control[BLACK] = popcount64(pos->controlled[BLACK]);
    res = control[WHITE] - control[BLACK];
#   ifdef DEBUG_EVAL
    log_f(2, "square control: W:%ld B:%ld eval=%ld (%.3f pawns)\n",
          control[WHITE], control[BLACK],
          res, (float)res/10);
#   endif

    /* 3) mobility: 5 mobility diff = 1 pawn
     */
    res = pos->mobility[WHITE] - pos->mobility[BLACK];
#   ifdef DEBUG_EVAL
    log_f(2, "mobility: W:%ld B:%ld eval=%ld (%.3f pawns)\n",
          pos->mobility[WHITE], pos->mobility[BLACK],
          res, (float)res/5);
#   endif

    res = material[WHITE] - material[BLACK] +
        (control[WHITE] - control[BLACK]) * 10 +
        (pos->mobility[WHITE] - pos->mobility[BLACK]) * 20;
#   ifdef DEBUG_EVAL
    log_f(2, "eval: %ld (%.3f pawns)\n",
          res, (float)res/100);
#   endif

    return res;
}

#ifdef BIN_eval
#include "fen.h"
#include "move.h"

int main(int ac, char**av)
{
    pos_t *pos;
    eval_t res;

    debug_init(2);
    piece_pool_init();
    moves_pool_init();
    pos = pos_create();

    if (ac == 1) {
        pos_startpos(pos);
    } else {
        fen2pos(pos, av[1]);
    }

    moves_gen(pos, OPPONENT(pos->turn), false);
    moves_gen(pos, pos->turn, true);
    pos_print(pos);
    pos_pieces_print(pos);
    res = eval(pos);
    printf("eval=%ld (%.3f pawns)\n", res, (float)res/100);
}
#endif
