/* eval.c - static position evaluation.
 *
 * Copyright (C) 2021 Bruno Raoult ("br")
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

#include <list.h>
#include <debug.h>

#include "eval.h"

eval_t eval(pos_t *pos)
{
    eval_t material[2] = {0};
    eval_t control[2] = {0};
    eval_t res = 0;

    //printf("val(%d-%c) = %lu\n", PAWN, P_LETTER(PAWN), P_VALUE(PAWN));
    //printf("val(%d-%c) = %lu\n", KNIGHT, P_LETTER(KNIGHT), P_VALUE(KNIGHT));
    //bitboard_print2(pos->bb[0][BB_PAWN], pos->bb[1][BB_PAWN]);
    //bitboard_print2(pos->bb[0][BB_QUEEN], pos->bb[1][BB_QUEEN]);
    /* 1) pieces value
     */
    for (uint color = 0; color < 2; ++color) {
        for (uint piece = PAWN; piece <= KING; piece <<= 1) {
            uint bb = PIECETOBB(piece);
#           ifdef DEBUG_EVAL
            log_f(2, "color=%u piece=%u bb=%u=%c count=%ul val=%ld\n",
                  color, piece, bb, P_LETTER(piece), popcount64(pos->bb[color][bb]),
                  P_VALUE(piece));
#           endif
            /* attention here */
            material[color] += popcount64(pos->bb[color][bb]) * P_VALUE(piece);
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

    /* 3) mobility: 10 mobility diff = 1 pawn
     */
    res = pos->mobility[WHITE] - pos->mobility[BLACK];
#   ifdef DEBUG_EVAL
    log_f(2, "mobility: W:%ud B:%ud eval=%ld (%.3f pawns)\n",
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

    debug_level_set(9);
    piece_pool_init();
    moves_pool_init();
    pos_pool_init();
    pos = pos_get();

    if (ac == 1) {
        pos_startpos(pos);
    } else {
        fen2pos(pos, av[1]);
    }

    pos_print(pos);
    pos_pieces_print(pos);

    moves_gen(pos, OPPONENT(pos->turn), false);
    moves_print(pos, M_PR_SEPARATE);
    //pos_print(pos);
    //pos_pieces_print(pos);
    moves_gen(pos, pos->turn, false);
    moves_print(pos, M_PR_SEPARATE);
    res = eval(pos);
    printf("eval=%ld (%.3f pawns)\n", res, (float)res/100);
}
#endif
