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

static pool_t *pieces_pool;

inline static char piece2char(piece_t p)
{
    piece_t piece = PIECE(p);
    char res;

    //printf("%#x p=%#x\n", p, PIECE(p));
    switch (piece) {
        case PAWN:
            res = CHAR_PAWN;
            break;
        case KNIGHT:
            res = CHAR_KNIGHT;
            break;
        case BISHOP:
            res = CHAR_BISHOP;
            break;
        case ROOK:
            res = CHAR_ROOK;
            break;
        case QUEEN:
            res = CHAR_QUEEN;
            break;
        case KING:
            res = CHAR_KING;
            break;
        default:
            res = CHAR_EMPTY;
    }
    return res;

}

void piece_list_print(struct list_head *list)
{
    struct list_head *p_cur, *tmp;
    piece_list_t *piece;
    int i = 0;

    list_for_each_safe(p_cur, tmp, list) {
        piece = list_entry(p_cur, piece_list_t, list);

        printf("%c%c%c ", piece2char(piece->piece),
               FILE2C(GET_F(piece->square)),
               RANK2C(GET_R(piece->square)));
        /*printf("\t%d: %s on %c%c\n", i,
               piece2string(piece->piece),
               FILE2C(GET_F(piece->square)),
               RANK2C(GET_R(piece->square)));
        */
        i++;
    }
    printf("Total pieces = %d\n", i);
}

void pieces_print_pos_pieces(pos_t *pos)
{
    printf("White pieces : \n\t");
    piece_list_print(&pos->pieces_white);
    printf("Black pieces : \n\t");
    piece_list_print(&pos->pieces_black);
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

    /* printf("%s: piece=%02x square=%02x\n", __func__, piece, square);
    printf("%s: Adding %s %s on %c%c\n",
           __func__,
           color? "Black": "White",
           piece2string(piece),
           FILE2C(GET_F(square)),
           RANK2C(GET_R(square)));
    */
    if ((new = pool_get(pieces_pool))) {
        //printf("color=%d addp=%p\n", COLOR(piece), &pos->pieces[COLOR(piece)]);
        list_add_tail(&new->list,
                 color? &pos->pieces_black: &pos->pieces_white);
        new->piece = piece;
        new->square = square;
        new->castle = 0;
        new-> value = pieces_values[PIECE(piece)];
    }

    return new;
}

#ifdef PIECEBIN
#include "fen.h"
int main(int ac, char**av)
{
    //size_t i;
    pos_t *pos;
    //piece_list_t *plist;

    pos = pos_create();
    piece_pool_init();


    if (ac == 1) {
        pos_startpos(pos);
    } else {
        fen2pos(pos, av[1]);
    }
    pos_print(pos);
    pieces_print_pos_pieces(pos);
}
#endif
