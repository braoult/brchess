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
#include "move.h"
#include "fen.h"
#include "piece.h"

static pool_t *pos_pool;

#define BYTE_PRINT "%c%c%c%c%c%c%c%c"
#define BYTE2BIN(b) ((b) & 0x01 ? '1' : '0'),  \
        ((b) & 0x02 ? '1' : '0'),              \
        ((b) & 0x04 ? '1' : '0'),              \
        ((b) & 0x08 ? '1' : '0'),              \
        ((b) & 0x10 ? '1' : '0'),              \
        ((b) & 0x20 ? '1' : '0'),              \
        ((b) & 0x40 ? '1' : '0'),              \
        ((b) & 0x80 ? '1' : '0')

inline void bitboard_print(bitboard_t bb)
{
    int i;
    printf("%#018lx\n", bb);
    for (i=56; i>=0; i-=8)
        printf("\t"BYTE_PRINT"\n",
               BYTE2BIN(bb>>i));
}

inline void bitboard_print2(bitboard_t bb1, bitboard_t bb2)
{
    int i;
    printf("\tW: %#018lx\tB: %#018lx\n", bb1, bb2);
    for (i=56; i>=0; i-=8)
        printf("\t"BYTE_PRINT"\t\t"BYTE_PRINT"\n",
               BYTE2BIN(bb1>>i),
               BYTE2BIN(bb2>>i));
}

void pos_pieces_print(pos_t *pos)
{
    printf("White pieces (%d): \t", popcount64(pos->occupied[WHITE]));
    piece_list_print(&pos->pieces[WHITE]);
    printf("Black pieces (%d): \t", popcount64(pos->occupied[BLACK]));
    piece_list_print(&pos->pieces[BLACK]);
}

/* void pos_print - Print position on stdout.
 * @pos:   Position address (pos_t * )
 *
 * Return: None.
 */
void pos_print(pos_t *pos)
{
    int rank, file;
    piece_t piece;
    board_t *board = pos->board;

    printf("  +---+---+---+---+---+---+---+---+\n");
    for (rank = 7; rank >= 0; --rank) {
        printf("%c |", rank + '1');
        for (file = 0; file < 8; ++file) {
            piece = board[SQ88(file, rank)].piece;
            printf(" %s |", P_CSYM(piece));
        }
        printf("\n  +---+---+---+---+---+---+---+---+\n");
    }
    printf("    A   B   C   D   E   F   G   H\n\n");
    printf("Turn: %s.\n", IS_WHITE(pos->turn) ? "white" : "black");
    printf("Kings: W:%c%c B:%c%c\n",
           FILE2C(GET_F(pos->king[WHITE])),
           RANK2C(GET_R(pos->king[WHITE])),
           FILE2C(GET_F(pos->king[BLACK])),
           RANK2C(GET_R(pos->king[BLACK])));
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
    printf("Squares controlled: W:%d B:%d\n", popcount64(pos->controlled[WHITE]),
           popcount64(pos->controlled[BLACK]));
    printf("Mobility: W:%u B:%u\n", pos->mobility[WHITE],
           pos->mobility[BLACK]);
    printf("Bitboards occupied :\n");
    bitboard_print2(pos->occupied[WHITE], pos->occupied[BLACK]);
    printf("Bitboards controlled :\n");
    bitboard_print2(pos->controlled[WHITE], pos->controlled[BLACK]);
}

pos_t *pos_clear(pos_t *pos)
{
    int file, rank;
    board_t *board = pos->board;

    for (file = 0; file < 8; ++file) {
        for (rank = 0; rank < 8; ++rank) {
            /*printf("file = %d rank = %d SQ88 = %#2x = %d addr=%p\n", file, rank,
                   SQ88(file, rank), SQ88(file, rank),
                   &board[SQ88(file, rank)].piece);
            */
            board[SQ88(file, rank)].piece = EMPTY;
        }
    }

    SET_WHITE(pos->turn);
    pos->castle = 0;
    pos->clock_50 = 0;
    pos->curmove = 0;
    pos->eval = 0;
    pos->en_passant = 0;
    pos->king[WHITE] = 0;
    pos->king[BLACK] = 0;
    pos->occupied[WHITE] = 0;
    pos->occupied[BLACK] = 0;
    pos->controlled[WHITE] = 0;
    pos->controlled[BLACK] = 0;
    pos->mobility[WHITE] = 0;
    pos->mobility[BLACK] = 0;
    /* remove pieces / moves */
    pieces_del(pos, WHITE);
    pieces_del(pos, BLACK);
    moves_del(pos);

    return pos;
}

pos_t *pos_startpos(pos_t *pos)
{
    static char *startfen="rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    return fen2pos(pos, startfen);
}

pos_t *pos_get()
{
    pos_t *pos = pool_get(pos_pool);
    if (pos) {
        INIT_LIST_HEAD(&pos->pieces[WHITE]);
        INIT_LIST_HEAD(&pos->pieces[BLACK]);
        INIT_LIST_HEAD(&pos->moves);
        pos_clear(pos);
    }
    return pos;
}

/* TODO: merge with pos_get - NULL for init, non null for duplicate */
pos_t *pos_dup(pos_t *pos)
{
    struct list_head *p_cur, *tmp, *piece_list;
    piece_list_t *oldpiece;
    board_t *board;
    pos_t *new = pool_get(pos_pool);

    if (new) {
        board = new->board;
        *new = *pos;
        INIT_LIST_HEAD(&new->pieces[WHITE]);
        INIT_LIST_HEAD(&new->pieces[BLACK]);
        INIT_LIST_HEAD(&new->moves);

        /* duplicate piece list */
        for (int color=0; color<2; ++color) {
            piece_list = &pos->pieces[color];     /* white/black piece list */

            list_for_each_safe(p_cur, tmp, piece_list) {
                oldpiece = list_entry(p_cur, piece_list_t, list);
                board[oldpiece->square].s_piece =
                    piece_add(new, oldpiece->piece, oldpiece->square);

            }
        }
    }
    return new;
}

pool_t *pos_pool_init()
{
    if (!pos_pool)
        pos_pool = pool_init("positions", 128, sizeof(pos_t));
    return pos_pool;
}

void pos_pool_stats()
{
    if (pos_pool)
        pool_stats(pos_pool);
}
