/* piece.c - piece list management.
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
#include "chessdefs.h"
#include "piece.h"
#include "ctype.h"
#include "debug.h"

static pool_t *pieces_pool;

struct piece_details piece_details[] = {
    [EMPTY] =  { ' ', ' ', " ", " ", "",       0 },
    [PAWN] =   { 'P', 'p', "♙", "♟", "Pawn",   PAWN_VALUE },
    [KNIGHT] = { 'N', 'n', "♘", "♞", "Knight", KNIGHT_VALUE },
    [BISHOP] = { 'B', 'b', "♗", "♝", "Bishop", BISHOP_VALUE },
    [ROOK] =   { 'R', 'r', "♖", "♜", "Rook",   ROOK_VALUE },
    [QUEEN] =  { 'Q', 'q', "♕", "♛", "Queen",  QUEEN_VALUE },
    [KING] =   { 'K', 'k', "♔", "♚", "King",   KING_VALUE }
};

void piece_list_print(struct list_head *list)
{
    struct list_head *p_cur, *tmp;
    piece_list_t *piece;
    int i = 0;

    list_for_each_safe(p_cur, tmp, list) {
        piece = list_entry(p_cur, piece_list_t, list);

        printf("%s%c%c ", P_SYM(piece->piece),
               FILE2C(GET_F(piece->square)),
               RANK2C(GET_R(piece->square)));
        i++;
    }
    printf("Total pieces = %d\n", i);
}

pool_t *piece_pool_init()
{
    if (!pieces_pool)
        pieces_pool = pool_init("pieces", 128, sizeof(piece_list_t));
    return pieces_pool;
}

static eval_t pieces_values[] = {
    [PAWN] = PAWN_VALUE,
    [KNIGHT] = KNIGHT_VALUE,
    [BISHOP] = BISHOP_VALUE,
    [ROOK] = ROOK_VALUE,
    [QUEEN] = QUEEN_VALUE,
    [KING] = KING_VALUE
};

piece_list_t *piece_add(pos_t *pos, piece_t piece, square_t square)
{
    piece_list_t *new;
    short color = COLOR(piece);

#   ifdef DEBUG_PIECE
    log_f(2, "piece=%02x square=%02x\n", piece, square);
    log_f(5, "Adding %s %s on %c%c\n", color? "Black": "White",
          piece2string(piece), FILE2C(GET_F(square)), RANK2C(GET_R(square)));
#   endif
    if ((new = pool_get(pieces_pool))) {
        list_add_tail(&new->list, &pos->pieces[color]);
        //color? &pos->pieces_black: &pos->pieces_white);
        new->piece = piece;
        new->square = square;
        new->castle = 0;
        new-> value = pieces_values[PIECE(piece)];
    }

    return new;
}

#ifdef BIN_piece
#include "fen.h"
int main(int ac, char**av)
{
    pos_t *pos;

    debug_init(5);
    pos = pos_create();
    piece_pool_init();

    if (ac == 1) {
        pos_startpos(pos);
    } else {
        fen2pos(pos, av[1]);
    }
    pos_print(pos);
    pos_pieces_print(pos);
}
#endif
