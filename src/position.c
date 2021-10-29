/* position.c - position management.
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
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "chessdefs.h"
#include "position.h"
#include "fen.h"

inline static char piece2char(unsigned char p)
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
    if (IS_BLACK(p))
        res = tolower(res);
    return res;

}

void pos_print(pos_t *pos)
{
    int rank, file;
    piece_t piece;
    board_t *board = pos->board;

    printf("  +---+---+---+---+---+---+---+---+\n");
    for (rank = 7; rank >= 0; --rank) {
        printf("%c |", rank + '1');
        for (file = 0; file < 8; ++file) {
            piece = board[SQ88(file, rank)]->piece;
            printf(" %c |", piece2char(piece));
        }
        printf("\n  +---+---+---+---+---+---+---+---+\n");
    }
    printf("    A   B   C   D   E   F   G   H\n\n");
    printf("Next move: %s.\n", IS_WHITE(pos->turn) ? "white" : "black");
    printf("Possible en-passant: [%#x] ", pos->en_passant);
    if (pos->en_passant == 0)
        printf("None.\n");
    else
        printf("%d %d = %c%c\n", GET_F(pos->en_passant),
               GET_R(pos->en_passant),
               FILE2C(GET_F(pos->en_passant)),
               RANK2C(GET_R(pos->en_passant)));

    printf("castle [%#x] : ", pos->castle);

    if (pos->castle & CASTLE_WK)
        printf("K");
    if (pos->castle & CASTLE_WQ)
        printf("Q");
    if (pos->castle & CASTLE_BK)
        printf("k");
    if (pos->castle & CASTLE_BQ)
        printf("q");

    printf("\n50 half-moves-rule = %d\n", pos->clock_50);
    printf("Current move = %d\n", pos->curmove);
}

pos_t *pos_init(pos_t *pos)
{
    int file, rank;
    board_t *board = pos->board;

    for (rank = 0; rank < 8; ++rank) {
        for (file = 0; file < 8; ++file) {
            printf("file = %d rank = %d SQ88 = %#x\n", file, rank, SQ88(file, rank));
            board[SQ88(file, rank)]->piece = EMPTY;
        }
    }

    pos->turn = WHITE;
    pos->castle = 0;
    pos->clock_50 = 0;
    pos->curmove = 0;
    pos->en_passant = 0;
    pos->en_passant = 0;

    return pos;
}

pos_t *pos_startpos(pos_t *pos)
{
    static char *startfen="rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    return fen2pos(pos, startfen);
}

pos_t *pos_create()
{
    pos_t *pos = malloc(sizeof(pos_t));
    if (pos) {
        pos->board = malloc(sizeof (board_t));
        if (pos->board)
            pos_init(pos);
        else {
            free(pos);
            pos = NULL;
        }
    }
    INIT_LIST_HEAD(&pos->w_pieces);
    INIT_LIST_HEAD(&pos->b_pieces);
    return pos;
}
