/* move.c - move management.
 *
 * Copyright (C) 2021-2024 Bruno Raoult ("br")
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
 * move_to_str() - get a move string
 * @dst: destination memory
 * @move: move
 * @flags: moves selection and display options.
 *
 * @dst should be large enough to contain move string + '\0' terminator.
 *
 * Possible flags are:
 * M_PR_CAPT:  print move if capture
 * M_PR_NCAPT: print move if non capture
 * M_PR_NUM:   print also move number
 * M_PR_LONG:  print long notation
 * M_PR_NL:    print a newline after move
 * M_PR_EVAL:  print move eval
 */
char *move_to_str(char *dst, const move_t move, __unused const int flags)
{
    if (move == MOVE_NONE) {
        strcpy(dst, "none");
    } else if (move == MOVE_NULL) {
        strcpy(dst, "null");
    } else {
        square_t from = move_from(move);
        square_t to   = move_to(move);
        int len;

        sprintf(dst, "%s%s%n", sq_to_string(from), sq_to_string(to), &len);

        if (is_promotion(move)) {
            piece_t promoted = (piece_t) move_promoted(move);
            sprintf(dst + len, "%s", piece_to_low(promoted));
        }
    }
    return dst;
}

/**
 * move_from_str() - create a move from an UCI move string
 * @str:  uci move string
 *
 * Only from/to squares and promotion information are filled.
 * To get a full move, @move_find_in_movelist() can be called,
 * with a list of moves to choose from.
 *
 * @return: partial move.
 */
move_t move_from_str(const char *str)
{
    move_t move;
    square_t from = sq_from_string(str);
    square_t to   = sq_from_string(str + 2);
    piece_type_t promoted = piece_t_from_char(*(str+4));

    if (promoted != NO_PIECE_TYPE) {              /* promotion */
        move = move_make_promote(from, to, promoted - 2);
    } else {
        move = move_make(from, to);
    }
    return move;
}

/**
 * move_find_in_movelist() - find a partial move in movelist.
 * @move:      move (may be partial)
 * @movelist:  &movelist_t to search
 *
 * Look for @target into @list, by comparing its from, to, and promotion
 * information.
 *
 * @return: move in @movelist if match found, MOVE_NONE otherwise.
 */
move_t move_find_in_movelist(move_t target, movelist_t *list)
{
    move_t *move = list->move, *last = move + list->nmoves;

    for (; move < last; ++move) {
        if (move_from(target) == move_from(*move) &&
            move_to(target)   == move_to(*move) &&
            move_promoted(target)   == move_promoted(*move))
            return *move;
    }
    return MOVE_NONE;
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
 * M_PR_NL:    print a newline after each move
 * M_PR_EVAL:  print move eval
 */
void moves_print(movelist_t *moves, __unused int flags)
{
    char str[16];
    //printf("%2d:", moves->nmoves);
    for (int m = 0; m < moves->nmoves; ++m)
        printf("%s ", move_to_str(str, moves->move[m], flags));
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
    piece_type_t prom1 = move_promoted(m1);
    piece_type_t prom2 = move_promoted(m2);
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
