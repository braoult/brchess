/* piece.c - piece list management.
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

#include <debug.h>
#include <pool.h>
#include <list.h>

#include "chessdefs.h"
#include "piece.h"
#include "board.h"
#include "position.h"

static pool_t *pieces_pool;

struct piece_details piece_details[] = {
    [E_EMPTY]  = { ' ', ' ', " ", " ", "",       0 },
    [E_PAWN]   = { 'P', 'p', "♙", "♟", "Pawn",   PAWN_VALUE },
    [E_KNIGHT] = { 'N', 'n', "♘", "♞", "Knight", KNIGHT_VALUE },
    [E_BISHOP] = { 'B', 'b', "♗", "♝", "Bishop", BISHOP_VALUE },
    [E_ROOK]   = { 'R', 'r', "♖", "♜", "Rook",   ROOK_VALUE },
    [E_QUEEN]  = { 'Q', 'q', "♕", "♛", "Queen",  QUEEN_VALUE },
    [E_KING]   = { 'K', 'k', "♔", "♚", "King",   KING_VALUE }
};

void piece_list_print(struct list_head *list)
{
    struct list_head *p_cur, *tmp;
    piece_list_t *piece;

    list_for_each_safe(p_cur, tmp, list) {
        piece = list_entry(p_cur, piece_list_t, list);

        printf("%s%c%c ", P_SYM(piece->piece),
               FILE2C(F88(piece->square)),
               RANK2C(R88(piece->square)));
    }
    printf("\n");
}

pool_t *piece_pool_init()
{
    if (!pieces_pool)
        pieces_pool = pool_create("pieces", 128, sizeof(piece_list_t));
    return pieces_pool;
}

void piece_pool_stats()
{
    if (pieces_pool)
        pool_stats(pieces_pool);
}

piece_list_t *piece_add(pos_t *pos, piece_t piece, square_t square)
{
    piece_list_t *new;
    short color = COLOR(piece);

#   ifdef DEBUG_PIECE
    log_f(3, "piece=%02x square=%02x\n", piece, square);
    log_f(5, "Adding %s %s on %c%c\n", color? "Black": "White",
          P_NAME(piece), FILE2C(F88(square)), RANK2C(R88(square)));
#   endif
    if ((new = pool_get(pieces_pool))) {
        /* first piece is always king */
        if (PIECE(piece) == KING)
            list_add(&new->list, &pos->pieces[color]);
        else
            list_add_tail(&new->list, &pos->pieces[color]);
        new->piece = piece;
        new->square = square;
        new->castle = 0;
        new-> value = piece_details[PIECE(piece)].value;
    }

    return new;
}

void piece_del(struct list_head *ptr)
{
    piece_list_t *piece = list_entry(ptr, piece_list_t, list);
#   ifdef DEBUG_PIECE
    log_f(3, "piece=%02x square=%02x\n", piece->piece, piece->square);
#   endif
    list_del(ptr);
    pool_add(pieces_pool, piece);
    return;
}

int pieces_del(pos_t *pos, short color)
{
    struct list_head *p_cur, *tmp, *head;
    int count = 0;

    head = &pos->pieces[color];

    list_for_each_safe(p_cur, tmp, head) {
        piece_del(p_cur);
        count++;
    }
#   ifdef DEBUG_PIECE
    log_f(3, "color=%d removed=%d\n", color, count);
#   endif
    return count;
}

#ifdef BIN_piece
#include "fen.h"
int main(int ac, char**av)
{
    pos_t *pos;
    printf("zobi\n");fflush(stdout);
    debug_init(6, stderr);
    log_f(5, "kfsjdhg\n");
    pos_pool_init();
    pos = pos_get();
    piece_pool_init();

    if (ac == 1) {
    printf("zoba\n");fflush(stdout);
        pos_startpos(pos);
    } else {
        fen2pos(pos, av[1]);
    }
    pos_print(pos);
    pos_pieces_print(pos);

    printf("0x1c = 11100 = C1-E1:\n");
    bitboard_print(0x1c);

    printf("0x70 = 111 = A1-C1\n");
    bitboard_print(0x70);

    printf("0x0e = 1110 = B1-D1\n");
    bitboard_print(0x0e);

    printf("0x60 = 1100000 = F1-G1\n");
    bitboard_print(0x60);

    printf("A1:\n");
    bitboard_print(A1);

    printf("1:\n");
    bitboard_print(1L);
    printf("H1:\n");
    bitboard_print(H1);
    printf("C1:\n");
    bitboard_print(C1);
    printf("D1:\n");
    bitboard_print(D1);
    printf("C1|D1:\n");
    bitboard_print(C1|D1);
    printf("H8:\n");
    bitboard_print(H8);
}
#endif
