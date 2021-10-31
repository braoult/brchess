/* move.c - move management.
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

#include <malloc.h>
#include <ctype.h>
#include "chessdefs.h"
#include "piece.h"
#include "move.h"
#include "list.h"

static pool_t *moves_pool;

static struct vector {
    unsigned char ndirs;
    char slide;
    char dir[8];
} vectors[] = {
    [KNIGHT] = { 8, 0, { -33, -31, -18, -14, 14, 18, 31, 33}},
    [BISHOP] = { 4, 1, { -15, -17, 15, 17}},
    [ROOK]   = { 4, 1, { -1, -16, 1, 16}},
    [QUEEN] =  { 8, 1, { -1, -16, 1, 16, -15, -17, 15, 17}},
    [KING] =   { 8, 0, { -1, -16, 1, 16, -15, -17, 15, 17}},
};

pool_t *moves_pool_init()
{
    if (!moves_pool)
        moves_pool = pool_init("moves", 128, sizeof(piece_list_t));
    return moves_pool;
}

void move_print(move_t *move)
{
    printf("%s%c%c", P_SYM(move->piece),
           FILE2C(GET_F(move->from)),
           RANK2C(GET_R(move->from)));
    if (move->taken)
        printf("x%s", P_SYM(move->taken));
    else
        printf("-");
    printf("%c%c",
           FILE2C(GET_F(move->to)),
           RANK2C(GET_R(move->to)));
    printf(" ");
}

void moves_print(pos_t *pos)
{
    struct list_head *p_cur, *tmp;
    move_t *move;
    int i = 0;

    printf("%s pseudo-moves: ", pos->turn == WHITE? "White": "Black");
    list_for_each_safe(p_cur, tmp, &pos->moves) {
        move = list_entry(p_cur, move_t, list);

        move_print(move);
        i++;
    }
    printf("Total moves = %d\n", i);
}

/* generate moves for non pawn pieces
 */
int pseudo_moves_get(pos_t *pos, piece_list_t *ppiece)
{
    piece_t piece = PIECE(ppiece->piece);
    struct vector *vector = vectors+piece;
    square_t square = ppiece->square;
    unsigned char ndirs = vector->ndirs;
    char slide = vector->slide;
    board_t *board = pos->board;
    char color = COLOR(piece);
    move_t *move;
    int count = 0;

    /*printf("%s: pos:%p piece:%d [%s] at %#04x[%c%c]\n", __func__, pos, piece,
           piece2string(piece),
           square,
           FILE2C(GET_F(square)),
           RANK2C(GET_R(square)));
    printf("\tvector=%ld ndirs=%d slide=%d\n", vector-vectors, ndirs, slide);
    */
    for (int curdir = 0; curdir < ndirs; ++curdir) {
        char dir = vector->dir[curdir];
        for (square_t new = square + dir; ; new = new + dir) {
            /* outside board */
            if (SQ88_NOK(new)) {
                //printf("\t\tskipping %04x (invalid square)\n", new);
                break;
            }
            //printf("\ttrying %c%c", FILE2C(GET_F(new)), RANK2C(GET_R(new)));
            /* own color on dest square */
            if (board[new].piece && COLOR(board[new].piece) == color) {
                //printf("\t\tskipping %04x (same color piece)\n", new);
                break;
            }

            /* we are sure the move is valid : we create move */
            if (!(move = pool_get(moves_pool))) {
                //printf("\t\tmem error\n");
                return count;
            }
            move->piece = piece;
            move->from = square;
            move->to = new;
            move->taken = board[new].piece;
            //move_print(move);
            list_add(&move->list, &pos->moves);
            count++;
            if (move->taken)
                break;
            if (!slide)
                break;
        }
    }
    return count;
}

int moves_get(pos_t *pos)
{
    struct list_head *p_cur, *tmp, *piece_list;
    piece_list_t *piece;
    int count = 0;

    piece_list = pos->turn == WHITE? &pos->pieces_white: &pos->pieces_black;

    list_for_each_safe(p_cur, tmp, piece_list) {
        piece = list_entry(p_cur, piece_list_t, list);
        if (PIECE(piece->piece) != PAWN)
            pseudo_moves_get(pos, piece);

        count++;
    }
    return count;
}

move_t *pseudo_moves_pawn(pos_t *pos)
{
    static short directions[] = {16};
    if (!pos && *directions)
        printf("for flycheck");
    return NULL;
}

/* unused
move_t *(*moves_fct[])(pos_t *) = {
    [PAWN] = pseudo_moves_pawn,
    [KNIGHT] = pseudo_moves_knight,
    [BISHOP] = pseudo_moves_bishop,
    [ROOK] = pseudo_moves_rook,
    [QUEEN] = pseudo_moves_queen,
    [KING] = pseudo_moves_king
};
*/

#ifdef MOVEBIN
#include "fen.h"
int main(int ac, char**av)
{
    pos_t *pos;

    pos = pos_create();
    piece_pool_init();
    moves_pool_init();


    if (ac == 1) {
        pos_startpos(pos);
    } else {
        fen2pos(pos, av[1]);
    }
    pos_print(pos);
    pieces_print_pos_pieces(pos);
    moves_get(pos);
    moves_print(pos);
}
#endif
