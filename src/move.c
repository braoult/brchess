/* move.c - move management.
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

#include <malloc.h>
#include <ctype.h>
#include <stdlib.h>

#include "chessdefs.h"
#include "move.h"
#include "position.h"


/*
 * /\**
 *  * move_print() - print a move
 *  * @movenum: move number
 *  * @move: &move to display
 *  * @flags: options to display
 *  *
 *  * Possible flags are:
 *  * M_PR_CAPT:  print move if capture
 *  * M_PR_NCAPT: print move if non capture
 *  * M_PR_NUM:   print also move number
 *  * M_PR_LONG:  print long notation
 *  * M_PR_NL:    print a newline after move
 *  * M_PR_EVAL:  print move eval
 *  *
 *  * @return: 0 if nothing printed, 1 otherwise
 *  *\/
 * int move_print(int movenum, move_t *move, move_flags_t flags)
 * {
 *     if ((flags & M_PR_CAPT) && !(move->flags & M_CAPTURE)) {
 * #       ifdef DEBUG_MOVE
 *         log_i(9, "skipping capture & %#04x\n", move->flags);
 * #       endif
 *         return 0;
 *     }
 *     if ((flags & M_PR_NCAPT) && (move->flags & M_CAPTURE)) {
 * #       ifdef DEBUG_MOVE
 *         log_i(9, "skipping !capture & %#04x\n", move->flags);
 * #       endif
 *         return 0;
 *     }
 *     if (flags & M_PR_NUM)
 *         log(1, "%d:", movenum);
 *     if (move->flags & M_CASTLE_K) {
 *         log(1, "O-O");
 *         goto end;
 *     } else if (move->flags & M_CASTLE_Q) {
 *         log(1, "O-O-O");
 *         goto end;
 *     } else {
 *         log(1, "%s%c%c", P_SYM(move->piece),
 *                FILE2C(F88(move->from)),
 *                RANK2C(R88(move->from)));
 *         if (move->flags & M_CAPTURE) {
 *             log(1, "x");
 *             if (flags & M_PR_LONG)
 *                 log(1, "%s", P_SYM(move->capture));
 *         } else {
 *             log(1, "-");
 *         }
 *         log(1, "%c%c",
 *                FILE2C(F88(move->to)),
 *                RANK2C(R88(move->to)));
 *         if (flags & M_PR_LONG && move->flags & M_EN_PASSANT)
 *             log(1, "e.p.");
 *         if (move->promotion)
 *             log(1, "=%s", P_SYM(move->promotion));
 *         if (flags & M_PR_EVAL)
 *             log(1, "[ev:%d] ", move->eval);
 *     end:
 *         log(1, " ");
 *     }
 *     if (flags & M_PR_NL)
 *         log(1, "\n");
 *     return 1;
 * }
 */

/**
 * move_str() - get a move string
 * @dst: destination memory
 * @move: move
 * @flags: moves selection and display options.
 *
 * Possible flags are:
 * M_PR_CAPT:  print move if capture
 * M_PR_NCAPT: print move if non capture
 * M_PR_NUM:   print also move number
 * M_PR_LONG:  print long notation
 * M_PR_NL:    print a newline after move
 * M_PR_EVAL:  print move eval
 */
char *move_str(char *dst, const move_t move, __unused const int flags)
{
    square_t from = move_from(move);
    square_t to   = move_to(move);
    int len;
    sprintf(dst, "%s-%s%n", sq_to_string(from), sq_to_string(to), &len);

    if (move & M_PROMOTION) {
        piece_t promoted = move_promoted(move);
        sprintf(dst + len, "=%s", piece_to_low(promoted));
    }
    return dst;
}


/**
 * moves_print() - print movelist moves.
 * @moves: &movelist_t moves list
 * @flags: moves selection and display options.
 *
 * Possible flags are:
 * M_PR_CAPT:  print move if capture
 * M_PR_NCAPT: print move if non capture
 * M_PR_NUM:   print also move number
 * M_PR_LONG:  print long notation
 * M_PR_NL:    print a newline after move
 * M_PR_EVAL:  print move eval
 */
void moves_print(movelist_t *moves, __unused int flags)
{
    char str[16];
    //printf("%2d:", moves->nmoves);
    for (int m = 0; m < moves->nmoves; ++m)
        printf("%s ", move_str(str, moves->move[m], flags));
    printf("\n");
}

static int _moves_cmp_bysquare(const void *p1, const void *p2)
{
    move_t m1 = *(move_t *)p1;
    move_t m2 = *(move_t *)p2;
    square_t f1 = move_from(m1);
    square_t t1 = move_to(m1);
    square_t f2 = move_from(m2);
    square_t t2 = move_to(m2);
    piece_t prom1 = move_promoted(m1);
    piece_t prom2 = move_promoted(m2);
    if (f1 < f2) return -1;
    if (f1 > f2) return 1;
    /* f1 == f2 */
    if (t1 < t2) return -1;
    if (t1 > t2) return 1;
    /* t1 == t2 */
    if (prom1 < prom2) return -1;
    if (prom1 > prom2) return 1;
    return 0;
}

/**
 * move_sort_by_sq() - sort moves list by from/to squares ascending
 * @moves: &movelist_t
 *
 * Used for perft, for easier comparison.
 */
void move_sort_by_sq(movelist_t *moves)
{
    qsort(moves->move, moves->nmoves, sizeof(move_t), _moves_cmp_bysquare);
}
