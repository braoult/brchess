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
    [KNIGHT] = { 8, 0, { -33, -31, -18, -14, 14,  18,  31, 33}},
    [BISHOP] = { 4, 1, { -15, -17, 15,  17}},
    [ROOK]   = { 4, 1, { -1,  -16, 1,   16}},
    [QUEEN] =  { 8, 1, { -1,  -16, 1,   16,  -15, -17, 15, 17}},
    [KING] =   { 8, 0, { -1,  -16, 1,   16,  -15, -17, 15, 17}},
};

pool_t *moves_pool_init()
{
    if (!moves_pool)
        moves_pool = pool_init("moves", 128, sizeof(piece_list_t));
    return moves_pool;
}

void move_print(move_t *move, move_flags_t flags)
{
    if (flags & M_PR_CAPT && !(move->flags & M_CAPTURE)) {
        // printf("capture & %#04x\n", move->flags);
        return;
    }
    if (flags & M_PR_NCAPT && move->flags & M_CAPTURE) {
        // printf("!capture & %#04x\n", move->flags);
        return;
    }
    if (move->flags & M_CASTLE_K) {
        printf("O-O");
        goto end;
    } else if (move->flags & M_CASTLE_Q) {
        printf("O-O-O");
        goto end;
    } else {
        printf("%s%c%c", P_SYM(move->piece),
               FILE2C(GET_F(move->from)),
               RANK2C(GET_R(move->from)));
        if (move->taken) {
            printf("x");
            if (flags & M_PR_LONG)
                printf("%s", P_SYM(move->taken));
        } else {
            printf("-");
        }
        printf("%c%c",
               FILE2C(GET_F(move->to)),
               RANK2C(GET_R(move->to)));
        if (flags & M_PR_LONG && move->flags & M_EN_PASSANT)
            printf("e.p.");
        if (move->promotion)
            printf("=%s", P_SYM(move->promotion));
        /*if (move->flags & M_CASTLE_K)
            printf("(O-O)");
        if (move->flags & M_CASTLE_Q)
            printf("(O-O-O)");
        */
    end:
        printf(" ");
    }
}

void moves_print(pos_t *pos, move_flags_t flags)
{
    struct list_head *p_cur, *tmp;
    move_t *move;
    int i = 0;
    move_flags_t details = flags & M_PR_LONG;

    printf("%s pseudo-moves:\n\t", pos->turn == WHITE? "White": "Black");
    if (! (flags & M_PR_SEPARATE)) {
        list_for_each_safe(p_cur, tmp, &pos->moves) {
            move = list_entry(p_cur, move_t, list);

            move_print(move, details);
            i++;
        }
        printf("\n\tTotal moves = %d\n", i);
    } else {
        printf("captures: ");
        list_for_each_safe(p_cur, tmp, &pos->moves) {
            move = list_entry(p_cur, move_t, list);

            move_print(move, details | M_PR_CAPT);
            i++;
        }
        printf("\n\tothers  : ");
        list_for_each_safe(p_cur, tmp, &pos->moves) {
            move = list_entry(p_cur, move_t, list);

            move_print(move, details | M_PR_NCAPT);
            i++;
        }
        printf("\n\tTotal moves = %d\n", i);
    }
}

inline static move_t *move_add(pos_t *pos, piece_t piece, square_t from,
                               square_t to)
{
    board_t *board = pos->board;
    move_t *move;

    if (!(move = pool_get(moves_pool)))
        return NULL;
    move->piece = piece;
    move->from = from;
    move->to = to;
    move->taken = board[to].piece;
    move->flags = M_NORMAL;
    if (move->taken)
        move->flags |= M_CAPTURE;
    list_add(&move->list, &pos->moves);
    return move;
}

/* TODO: return nmoves */
inline static move_t *move_pawn_add(pos_t *pos, piece_t piece, square_t from,
                                    square_t to, unsigned char rank7)
{
    //board_t *board = pos->board;
    move_t *move;
    piece_t promote;
    unsigned char color = COLOR(piece);

    if (RANK88(from) == rank7) {                  /* promotion */
        for (promote = QUEEN; promote > PAWN; promote >>= 1) {
            if ((move = move_add(pos, piece, from, to))) {
                move->flags |= M_PROMOTION;
                move->promotion = promote | color;
            }
        }
    } else {
        move = move_add(pos, piece, from, to);
    }
    return move;
}

/* pawn moves. We do not test for valid destination square here,
 * assuming position is valid. Is that correct ?
 */
int pseudo_moves_pawn(pos_t *pos, piece_list_t *ppiece)
{
    piece_t piece = PIECE(ppiece->piece);
    square_t square = ppiece->square, new;
    unsigned char color = COLOR(ppiece->piece);
    board_t *board = pos->board;
    unsigned char rank2, rank7, rank5;
    move_t *move = NULL;
    char dir;
    int count = 0;

    /* setup direction */
    if (color == WHITE) {
        dir = 1;
        rank2 = 1;
        rank7 = 6;
        rank5 = 4;
    } else {
        dir = -1;
        rank2 = 6;
        rank7 = 1;
        rank5 = 3;
    }

    /* normal push. We do not test for valid destination square here,
     * assuming position is valid. Is that correct ?
     */
    //moves_print(pos);
    new = square + dir * 16;
    if (!board[new].piece) {
        //printf("pushing pawn %#04x\n", square);
        if ((move = move_pawn_add(pos, piece | color, square, new, rank7)))
            count++;
        //moves_print(pos);

        /* push 2 squares */
        if (move && RANK88(square) == rank2) {
            new += dir * 16;
            if (SQ88_OK(new) && !board[new].piece) {
                //printf("pushing pawn %#04x 2 squares\n", square);
                if ((move = move_pawn_add(pos, piece | color, square, new, rank7)))
                    count++;
            }
        }
    }

    /* en passant */
    if (pos->en_passant && RANK88(square) == rank5) {
        unsigned char ep_file = FILE88(pos->en_passant);
        unsigned char sq_file = FILE88(square);

        //printf("possible en passant on rank %#x (current = %#0x)\n", ep_file,
        //    sq_file);

        if (sq_file == ep_file - 1 || sq_file == ep_file + 1) {
            square_t taken = board[SQUARE(ep_file, rank5)].piece;
            move = move_pawn_add(pos, piece | color , square, pos->en_passant, rank7);
            count++;
            move->flags |= M_EN_PASSANT | M_CAPTURE;
            move->taken = taken;
            count++;
        }

    }

    /* capture */
    int two=0;
    for (new = square + dir * 15; two < 2; new = square + dir * 17, two++) {
        //for (new = square + dir * 15; new <= square + dir * 17; new += 2) {
        //printf("pawn capture %#04x\n", square);
        if (SQ88_NOK(new))
            continue;
        if (board[new].piece && COLOR(board[new].piece) != color) {
            if ((move = move_pawn_add(pos, piece | color, square, new, rank7)))
                count++;
        }
    }
    return count;
}

int pseudo_moves_castle(pos_t *pos)
{
    unsigned char color = pos->turn;
    board_t *board = pos->board;
    unsigned char rank1, castle_K, castle_Q;
    move_t *move = NULL;
    unsigned short count=0;

    if (color == WHITE) {
        rank1 = 0;
        castle_K = pos->castle & CASTLE_WK;
        castle_Q = pos->castle & CASTLE_WQ;
    } else {
        rank1 = 7;
        castle_K = pos->castle & CASTLE_BK;
        castle_Q = pos->castle & CASTLE_BQ;
    }

    if (castle_K) {
        if (!(board[SQUARE(5, rank1)].piece ||
              board[SQUARE(6, rank1)].piece)) {
            move = move_add(pos, board[SQUARE(4, rank1)].piece,
                            SQUARE(4, rank1), SQUARE(6, rank1));
            if (move) {
                move->flags |= M_CASTLE_K;
            }
            count++;
        }
    }
    if (castle_Q) {
        if (!(board[SQUARE(1, rank1)].piece ||
              board[SQUARE(2, rank1)].piece ||
              board[SQUARE(3, rank1)].piece )) {
            move = move_add(pos, board[SQUARE(4, rank1)].piece,
                            SQUARE(4, rank1), SQUARE(2, rank1));
            if (move) {
                move->flags |= M_CASTLE_Q;
            }
            count++;
        }
    }
    return count;
}

/* general rule moves for non pawn pieces
 */
int pseudo_moves_gen(pos_t *pos, piece_list_t *ppiece)
{
    piece_t piece = PIECE(ppiece->piece);
    struct vector *vector = vectors+piece;
    square_t square = ppiece->square;
    unsigned char ndirs = vector->ndirs;
    char slide = vector->slide;
    board_t *board = pos->board;
    unsigned char color = COLOR(ppiece->piece);
    move_t *move;
    int count = 0;

    /*printf("square=%#04x piece=%#04x color = %s\n", square, piece, color==WHITE? "white": "black");
      printf("%s: pos:%p piece:%d [%s] at %#04x[%c%c]\n", __func__, pos, piece,
           P_NAME(piece),
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
            // printf("\ttrying %c%c", FILE2C(GET_F(new)), RANK2C(GET_R(new)));
            /* own color on dest square */
            /*if (board[new].piece) {
                printf("color=%d color2=%d\n", color, COLOR(board[new].piece));
                }*/
            if (board[new].piece && COLOR(board[new].piece) == color) {
                //printf("\t\tskipping %04x (same color piece)\n", new);
                break;
            }

            /* we are sure the move is valid : we create move */
            if ((move = move_add(pos, piece, square, new)))
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

    pseudo_moves_castle(pos);
    list_for_each_safe(p_cur, tmp, piece_list) {
        piece = list_entry(p_cur, piece_list_t, list);
        if (PIECE(piece->piece) != PAWN)
            pseudo_moves_gen(pos, piece);
        else
            pseudo_moves_pawn(pos, piece);

        count++;
    }
    return count;
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

#ifdef BIN_move
#include "fen.h"
#include "debug.h"
int main(int ac, char**av)
{
    pos_t *pos;

    debug_init(1);
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
    moves_print(pos, M_PR_SEPARATE);
}
#endif
