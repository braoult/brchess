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

#include <br.h>
#include <list.h>
#include <debug.h>

#include "chessdefs.h"
#include "board.h"
#include "piece.h"
#include "move.h"

static pool_t *moves_pool;

static struct vector {
    unsigned char ndirs;
    char          slide;
    char          dir[8];
} vectors[] = {
    [KNIGHT] = { 8, 0, { -33, -31, -18, -14, 14,  18,  31, 33 }},
    [BISHOP] = { 4, 1, { -15, -17, 15,  17 }},
    [ROOK]   = { 4, 1, { -1,  -16, 1,   16 }},
    [QUEEN]  = { 8, 1, { -1,  -16, 1,   16,  -15, -17, 15, 17 }},
    [KING]   = { 8, 0, { -1,  -16, 1,   16,  -15, -17, 15, 17 }}
};

/* squares needed to be empty & not controlled by opponent for castle.
 * For black castle, same values 7 rows higher (>> 7*8)
 */
static struct can_castle {
    bitboard_t controlled[2];
    bitboard_t occupied[2];
} castle_squares[2] = {
    /*  Queen side      King side           Queen side      King side */
    { { C1|D1|E1,       E1|F1|G1,      }, { B1|C1|D1,       F1|G1 } },
    { { C8|D8|E8,       E8|F8|G8,      }, { B8|C8|D8,       F8|G8 } }
};

pool_t *moves_pool_init()
{
    if (!moves_pool)
        moves_pool = pool_create("moves", 128, sizeof(move_t));
    return moves_pool;
}

void moves_pool_stats()
{
    if (moves_pool)
        pool_stats(moves_pool);
}

int move_print(move_t *move, move_flags_t flags)
{
    if (flags & M_PR_CAPT && !(move->flags & M_CAPTURE)) {
#       ifdef DEBUG_MOVE
        log_i(9, "skipping capture & %#04x\n", move->flags);
#       endif
        return 0;
    }
    if (flags & M_PR_NCAPT && move->flags & M_CAPTURE) {
#       ifdef DEBUG_MOVE
        log_i(9, "skipping !capture & %#04x\n", move->flags);
#       endif
        return 0;
    }
    if (move->flags & M_CASTLE_K) {
        printf("O-O");
        goto end;
    } else if (move->flags & M_CASTLE_Q) {
        printf("O-O-O");
        goto end;
    } else {
        printf("%s%c%c", P_SYM(move->piece),
               FILE2C(F88(move->from)),
               RANK2C(R88(move->from)));
        if (move->taken) {
            printf("x");
            if (flags & M_PR_LONG)
                printf("%s", P_SYM(move->taken));
        } else {
            printf("-");
        }
        printf("%c%c",
               FILE2C(F88(move->to)),
               RANK2C(R88(move->to)));
        if (flags & M_PR_LONG && move->flags & M_EN_PASSANT)
            printf("e.p.");
        if (move->promotion)
            printf("=%s", P_SYM(move->promotion));
    end:
        printf(" ");
    }
    return 1;
}

void moves_print(pos_t *pos, move_flags_t flags)
{
    struct list_head *p_cur, *tmp;
    move_t *move;
    move_flags_t details = flags & M_PR_LONG;

    for (int color=WHITE; color <= BLACK; ++color) {
        int verif = 0;
        printf("%s pseudo-moves:\n\t", color == WHITE? "White": "Black");
        if (! (flags & M_PR_SEPARATE)) {
            list_for_each_safe(p_cur, tmp, &pos->moves[color]) {
                move = list_entry(p_cur, move_t, list);
                verif += move_print(move, details);
            }
        } else {
            printf("captures: ");
            list_for_each_safe(p_cur, tmp, &pos->moves[color]) {
                move = list_entry(p_cur, move_t, list);
                verif += move_print(move, details | M_PR_CAPT);
            }
            printf("\n\tothers  : ");
            list_for_each_safe(p_cur, tmp, &pos->moves[color]) {
                move = list_entry(p_cur, move_t, list);
                verif += move_print(move, details | M_PR_NCAPT);
            }
        }
        printf("\n\tTotal moves = %d mobility=%u\n", verif, pos->mobility[color]);
    }
}

static move_t *move_add(pos_t *pos, piece_t piece, square_t from,
                        square_t to)
{
    //pos_t *newpos;
    board_t *board = pos->board;
    move_t *move;
    int color = COLOR(piece);

#   ifdef DEBUG_MOVE
    log_i(3, "piece_color=%d turn=%d from=%c%c to=%c%c\n",
          color, pos->turn,
          FILE2C(F88(from)),
          RANK2C(R88(from)),
          FILE2C(F88(to)),
          RANK2C(R88(to)));
#   endif
    //if (COLOR(piece) != pos->turn)
    //    return NULL;
    /* invalid position if opponent king is attacked
     */
    if (board[to].piece & KING) {
#       ifdef DEBUG_MOVE
        log_i(2, "invalid position: opponent king [%c%c] is in check.\n",
              FILE2C(F88(to)), RANK2C(R88(to)));
#       endif
        return NULL;
    }
    if (!(move = pool_get(moves_pool)))
        return NULL;
    list_add(&move->list, &pos->moves[color]);

    move->piece = piece;
    move->from = from;
    move->to = to;
    move->taken = board[to].piece;
    move->flags = M_NORMAL;
    move->newpos = pos_dup(pos);
    //newpos = move->newpos;
    //SET_COLOR(newpos->turn, OPPONENT(move->piece));
    //newpos->turn = OPPONENT(newpos->turn);
    if (move->taken) {
        move->flags |= M_CAPTURE;
        /* remove taken piece from new position piece list
         * this does not apply for en passant
         */
        //piece_del(&newpos->board[to].s_piece->list);
        /* remove occupied bitboard */
        //newpos->occupied[OPPONENT(COLOR(piece))] ^= SQ88_2_BB(to);
    }
    /* always make "to" the piece square in new position */
    //newpos->board[to] = newpos->board[from];
    /* fix dest square */
    //newpos->board[to].s_piece->square = to;
    /* replace old occupied bitboard by new one */
    //newpos->occupied[COLOR(piece)] ^= SQ88_2_BB(from);
    //newpos->occupied[COLOR(piece)] |= SQ88_2_BB(to);

    /* always make "from" square empty */
    //newpos->board[from].piece = 0;
    //newpos->board[from].s_piece = NULL;

#   ifdef DEBUG_MOVE
    log_i(3, "added move from %c%c to %c%c\n",
          FILE2C(F88(move->from)), RANK2C(R88(move->from)),
          FILE2C(F88(move->to)), RANK2C(R88(move->to)));
#   endif
    return move;
}

void move_del(struct list_head *ptr)
{
    move_t *move = list_entry(ptr, move_t, list);

#   ifdef DEBUG_MOVE
    log_i(3, "delete move from %c%c to %c%c\n",
          FILE2C(F88(move->from)), RANK2C(R88(move->from)),
          FILE2C(F88(move->to)), RANK2C(R88(move->to)));
#   endif

    /* TODO: remove move->pos if non null */
    if (move->newpos) {
        pos_clear(move->newpos);
    }
    list_del(ptr);
    pool_add(moves_pool, move);
    return;
}

int moves_del(pos_t *pos)
{
    struct list_head *p_cur, *tmp, *head;
    int count = 0;

    for (int color = WHITE; color <= BLACK; ++color) {
        head = &pos->moves[color];

        list_for_each_safe(p_cur, tmp, head) {
            move_del(p_cur);
            count++;
        }
    }
#   ifdef DEBUG_PIECE
    log_f(3, "removed=%d\n", count);
#   endif
    return count;
}

/* TODO: return nmoves */
static move_t *move_pawn_add(pos_t *pos, piece_t piece, square_t from,
                             square_t to, unsigned char rank7)
{
    move_t *move;
    piece_t promote;
    unsigned char color = COLOR(piece);
    pos_t *newpos;

    //if (color != pos->turn)
    //    return NULL;
    if (R88(from) == rank7) {                  /* promotion */
        for (promote = QUEEN; promote > PAWN; promote >>= 1) {
            if ((move = move_add(pos, piece, from, to))) {
                move->flags |= M_PROMOTION;
                move->promotion = promote | color;
                /* fix piece on board and piece list */
                newpos = move->newpos;
                newpos->board[to].piece = promote|color;
                newpos->board[to].s_piece->piece = piece|color;
                newpos->board[to].s_piece->value =  piece_details[PIECE(piece)].value;

                //piece_del(&newpos->board[from].s_piece);
            }
        }
    } else {
        move = move_add(pos, piece, from, to);
        newpos = move->newpos;

    }
    return move;
}

/* pawn moves. We do not test for valid destination square here,
 * assuming position is valid. Is that correct ?
 */
int pseudo_moves_pawn(pos_t *pos, piece_list_t *ppiece, bool doit)
{
    piece_t piece = PIECE(ppiece->piece);
    unsigned char color = COLOR(ppiece->piece);
    square_t square = ppiece->square, new;
    board_t *board = pos->board;
    unsigned char rank2, rank7, rank5;
    move_t *move = NULL;
    char dir;
    int count = 0;

    /* setup direction */
    if (IS_WHITE(color)) {
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

#   ifdef DEBUG_MOVE
    log_f(2, "pos:%p turn:%s piece:%d [%s %s] dir:%d at %#04x[%c%c]\n",
          pos,
          IS_WHITE(pos->turn)? "white": "black",
          piece,
          IS_WHITE(color)? "white": "black",
          P_NAME(piece),
          dir,
          square,
          FILE2C(F88(square)), RANK2C(R88(square)));
#   endif

    /* normal push. We do not test for valid destination square here,
     * assuming position is valid. Is that correct ?
     */
    new = square + dir * 16;
    if (!board[new].piece) {
#       ifdef DEBUG_MOVE
        log_i(4, "pushing pawn %#04x\n", square);
#       endif
        //log_f(4, "pawn move mobility\n");
        pos->mobility[color]++;
        count++;
        if (doit)
            move_pawn_add(pos, piece | color, square, new, rank7);
        /* push 2 squares */
        log(4, "R88(%#x)=%d R2=%d \n", square, R88(square), rank2);
        if (R88(square) == rank2) {
            new += dir * 16;
            if (SQ88_OK(new) && !board[new].piece) {
#               ifdef DEBUG_MOVE
                log_i(4, "pushing pawn %#04x 2 squares\n", square);
#               endif
                //log_f(2, "pawn move2 mobility\n");
                pos->mobility[color]++;
                count++;
                if (doit)
                    move_pawn_add(pos, piece | color, square, new, rank7);
            }
        }
    }

    /* en passant - not accounted for mobility. Correct ? */
    if (pos->en_passant && R88(square) == rank5) {
        unsigned char ep_file = F88(pos->en_passant);
        unsigned char sq_file = F88(square);

#       ifdef DEBUG_MOVE
        log_i(4, "possible en passant on rank %#x (current = %#0x)\n", ep_file,
              sq_file);
#       endif
        if (sq_file == ep_file - 1 || sq_file == ep_file + 1) {
            square_t t_square = SQ88(ep_file, rank5); /* taken pawn square */
            piece_t taken = board[t_square].piece;
            move = move_pawn_add(pos, piece | color , square, pos->en_passant, rank7);
            move->flags |= M_EN_PASSANT | M_CAPTURE;
            move->taken = taken;

            /* remove taken pawn from board */
            piece_del(&move->newpos->board[t_square].s_piece->list);
            move->newpos->board[t_square].piece = 0;
            move->newpos->board[t_square].s_piece = NULL;

            pos->mobility[color]++;
            count++;
        }

    }

    /* capture */
    int two=0;
    for (new = square + dir * 15; two < 2; new = square + dir * 17, two++) {
#       ifdef DEBUG_MOVE
        log_i(4, "pawn capture %#04x %#04x\n", square, new);
#       endif
        if (SQ88_NOK(new))
            continue;
        pos->controlled[color] |= SQ88_2_BB(new);
        if (board[new].piece && COLOR(board[new].piece) != color) {
            //log_f(2, "pawn capture mobility\n");
            pos->mobility[color]++;
            count++;
            if (doit)
                move_pawn_add(pos, piece | color, square, new, rank7);
        }
    }
#   ifdef DEBUG_MOVE
    log_f(2, "pos:%p turn:%s piece:%d [%s %s] dir:%d at %#04x[%c%c] count=%d\n",
          pos,
          IS_WHITE(pos->turn)? "white": "black",
          piece,
          IS_WHITE(color)? "white": "black",
          P_NAME(piece),
          dir,
          square,
          FILE2C(F88(square)), RANK2C(R88(square)), count);
#   endif
    return count;
}

int pseudo_moves_castle(pos_t *pos, bool color, bool doit)
{
    //unsigned char color = pos->turn;
    board_t *board = pos->board;
    unsigned char rank1, castle_K, castle_Q;
    move_t *move = NULL;
    unsigned short count=0;
    struct can_castle *can_castle;
    bitboard_t controlled;
    bitboard_t occupied = pos->occupied[WHITE] | pos->occupied[BLACK];
    pos_t *newpos;

#   ifdef DEBUG_MOVE
    log_f(2, "pos:%p turn:%s color:%s\n",
          pos,
          IS_WHITE(pos->turn)? "white": "black",
          IS_WHITE(color)? "white": "black");
#   endif

    if (IS_WHITE(color)) {
        rank1 = 0;
        castle_K = pos->castle & CASTLE_WK;
        castle_Q = pos->castle & CASTLE_WQ;
        can_castle = castle_squares+WHITE;
        controlled = pos->controlled[BLACK];
    } else {
        rank1 = 7;
        castle_K = pos->castle & CASTLE_BK;
        castle_Q = pos->castle & CASTLE_BQ;
        can_castle = castle_squares+BLACK;
        controlled = pos->controlled[WHITE];
    }
    if (castle_K) {
        if (occupied & can_castle->occupied[1]) {
#           ifdef DEBUG_MOVE
            log(5, "Cannot castle K side: occupied\n");
#           endif
            goto next;
        }
        if (controlled & can_castle->controlled[1]) {
#           ifdef DEBUG_MOVE
            log(5, "Cannot castle K side: controlled\n");
#           endif
            goto next;
        }
        pos->mobility[color]++;
        count++;
        if (doit) {
            move = move_add(pos, board[SQ88(4, rank1)].piece,
                            SQ88(4, rank1), SQ88(6, rank1));
            if (move) {
                newpos = move->newpos;
                move->flags |= M_CASTLE_K;

                /* move King rook to column F */
                newpos->board[SQ88(5, rank1)] = newpos->board[SQ88(7, rank1)];
                SETF88(newpos->board[SQ88(5, rank1)].s_piece->square, 5);
                newpos->board[SQ88(7, rank1)].piece =  0;
                newpos->board[SQ88(7, rank1)].s_piece = NULL;

            }
        }
    }

next:
    if (castle_Q) {
        if (occupied & can_castle->occupied[0]) {
#           ifdef DEBUG_MOVE
            log(5, "Cannot castle Q side: occupied\n");
#           endif
            goto end;
        }
        if (controlled & can_castle->controlled[0]) {
#           ifdef DEBUG_MOVE
            log(5, "Cannot castle Q side: controlled\n");
#           endif
            goto end;
        }
        pos->mobility[color]++;
        count++;
        if (doit) {
            move = move_add(pos, board[SQ88(4, rank1)].piece,
                            SQ88(4, rank1), SQ88(2, rank1));
            if (move) {
                newpos = move->newpos;
                move->flags |= M_CASTLE_Q;
                /* move King rook to column F */
                newpos->board[SQ88(3, rank1)] = newpos->board[SQ88(0, rank1)];
                SETF88(newpos->board[SQ88(3, rank1)].s_piece->square, 3);
                newpos->board[SQ88(0, rank1)].piece =  0;
                newpos->board[SQ88(0, rank1)].s_piece = NULL;
            }
        }
    }
end:
    return count;
}

/* general rule moves for non pawn pieces
 */
int pseudo_moves_gen(pos_t *pos, piece_list_t *ppiece, bool doit)
{
    piece_t piece = PIECE(ppiece->piece);
    unsigned char color = COLOR(ppiece->piece);
    struct vector *vector = vectors+piece;
    square_t square = ppiece->square;
    unsigned char ndirs = vector->ndirs;
    char slide = vector->slide;
    board_t *board = pos->board;
    //move_t *move;
    int count = 0;
    u64 bb_new;

#   ifdef DEBUG_MOVE
    log_f(2, "pos:%p turn:%s piece:%d [%s %s] at %#04x[%c%c]\n",
          pos,
          IS_WHITE(pos->turn)? "white": "black",
          piece,
          IS_WHITE(color)? "white": "black",
          P_NAME(piece),
          square,
          FILE2C(F88(square)), RANK2C(R88(square)));
    log_i(5, "vector=%ld ndirs=%d slide=%d\n", vector-vectors, ndirs, slide);
#   endif

    for (int curdir = 0; curdir < ndirs; ++curdir) {
        char dir = vector->dir[curdir];
        for (square_t new = square + dir; ; new = new + dir) {
            /* outside board */
            if (SQ88_NOK(new)) {
#               ifdef DEBUG_MOVE
                log_i(4,"skipping %04x (invalid square)\n", new);
#               endif
                break;
            }
            bb_new = SQ88_2_BB(new);
#           ifdef DEBUG_MOVE
            log_i(2, "trying %#x=%c%c bb=%#lx\n",
                  new, FILE2C(F88(new)), RANK2C(R88(new)),
                  bb_new);
            //bitboard_print(new_bitboard);
#           endif

            pos->controlled[color] |= bb_new;;

            /* king: do not move to opponent controlled square */
            if (piece == KING && pos->controlled[OPPONENT(color)] & bb_new) {
#               ifdef DEBUG_MOVE
                log_i(2, "%s king cannot move to %c%c\n",
                      IS_WHITE(color)? "white": "black",
                      FILE2C(F88(new)), RANK2C(R88(new)));
#               endif
                break;
            }

            if (bb_new & pos->occupied[color]) {
                //bitboard_print(pos->occupied[color]);
                //bitboard_print(pos->occupied[OPPONENT(color)]);
#               ifdef DEBUG_MOVE
                log_i(2, "BB: skipping %#lx [%c%c] (same color piece)\n",
                      bb_new, FILE2C(F88(new)), RANK2C(R88(new)));
#               endif
                break;
            }

            /* we are sure the move is valid : we create move */
            //log_f(2, "piece mobility\n");
            pos->mobility[color]++;
            count++;
            if (doit) {
                //move = move_add(pos, ppiece->piece, square, new);
                move_add(pos, ppiece->piece, square, new);
            }
            if (board[new].piece) {               /* stopper move */
                break;
            }
            if (!slide)
                break;
        }
    }
#   ifdef DEBUG_MOVE
    log_f(2, "pos:%p turn:%s piece:%d [%s %s] at %#04x[%c%c] count=%d\n",
          pos,
          IS_WHITE(pos->turn)? "white": "black",
          piece,
          IS_WHITE(color)? "white": "black",
          P_NAME(piece),
          square,
          FILE2C(F88(square)), RANK2C(R88(square)), count);
#   endif
    return count;
}

int moves_gen(pos_t *pos, bool color, bool doit)
{
    struct list_head *p_cur, *tmp, *piece_list;
    piece_list_t *piece;
    int count = 0;

#   ifdef DEBUG_MOVE
    log_f(2, "color:%s doit:%d\n", color? "Black": "White", doit);
#   endif
    piece_list = &pos->pieces[color];

    pos->mobility[color] = 0;
    pos->controlled[color] = 0;
    count += pseudo_moves_castle(pos, color, doit);
    list_for_each_safe(p_cur, tmp, piece_list) {
        piece = list_entry(p_cur, piece_list_t, list);
        if (PIECE(piece->piece) != PAWN)
            count += pseudo_moves_gen(pos, piece, doit);
        else
            count += pseudo_moves_pawn(pos, piece, doit);

        count++;
    }
    return count;
}

/* note: for now, a new pos is generated */
struct pos *move_do(pos_t *pos, move_t *move)
{
#   ifdef DEBUG_MOVE
    log_f(3, "++++++++++");
    move_print(move, 0);
#   endif
    pos_t *newpos = pos_dup(pos);
    piece_t piece = move->piece;
    int color = COLOR(piece);
    square_t from = move->from, to = move->to;

    /* todo: en passant
     */
    SET_COLOR(pos->turn, OPPONENT(color));        /* pos color */

    if (move->taken || PIECE(piece) == PAWN)      /* 50 moves */
        newpos->clock_50 = 0;
    else
        newpos->clock_50++;
    if (move->taken) {                            /*  */
        piece_del(&newpos->board[to].s_piece->list);
        newpos->occupied[OPPONENT(color)] ^= SQ88_2_BB(to);
    }
    newpos->board[to] = newpos->board[from];
    /* fix dest square */
    newpos->board[to].s_piece->square = to;
    /* replace old occupied bitboard by new one */
    newpos->occupied[color] ^= SQ88_2_BB(from);
    newpos->occupied[color] |= SQ88_2_BB(to);

    /* always make "from" square empty */
    newpos->board[from].piece = 0;
    newpos->board[from].s_piece = NULL;

    return 0;
}

void move_undo(pos_t *pos, __unused move_t *move)
{
    pos_clear(pos);
}

#ifdef BIN_move
#include "fen.h"
int main(int ac, char**av)
{
    pos_t *pos;

    debug_init(5, stderr);
    piece_pool_init();
    moves_pool_init();
    pos_pool_init();
    pos = pos_get();

    if (ac == 1) {
        fen2pos(pos, "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2");
        //pos_startpos(pos);
    } else {
        fen2pos(pos, av[1]);
    }
    //printf("turn = %d opponent = %d\n", pos->turn, OPPONENT(pos->turn));
    moves_gen(pos, WHITE, false);
    moves_gen(pos, BLACK, false);
    pos_print(pos);
    pos_pieces_print(pos);
    moves_print(pos, M_PR_SEPARATE);

    //bitboard_print2(castle_squares[0].controlled, castle_squares[1].controlled);
    //bitboard_print2(castle_squares[0].occupied, castle_squares[1].occupied);
}
#endif  /* BIN_move */
