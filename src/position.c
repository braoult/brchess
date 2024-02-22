/* position.c - position management.
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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "brlib.h"
#include "bitops.h"

#include "chessdefs.h"
#include "position.h"
#include "bitboard.h"
#include "fen.h"
#include "piece.h"
#include "util.h"
//#include "eval.h"

//static pool_t *pos_pool;

//const char *rankstr = "12345678";
//const char *filestr = "ABCDEFGH";

/****************************************************
 * #define BYTE_PRINT "%c%c%c%c%c%c%c%c"            *
 * #define BYTE2BIN(b) ((b) & 0x01 ? '1' : '0'),  \ *
 *         ((b) & 0x02 ? '1' : '0'),              \ *
 *         ((b) & 0x04 ? '1' : '0'),              \ *
 *         ((b) & 0x08 ? '1' : '0'),              \ *
 *         ((b) & 0x10 ? '1' : '0'),              \ *
 *         ((b) & 0x20 ? '1' : '0'),              \ *
 *         ((b) & 0x40 ? '1' : '0'),              \ *
 *         ((b) & 0x80 ? '1' : '0')                 *
 ****************************************************/

/**
 * pos_new() - allocate a new position
 *
 * position is not initialized
 */
pos_t *pos_new(void)
{
    return safe_malloc(sizeof(pos_t));
}

/**
 * pos_dup() - duplicate a position.
 * @pos: &position to duplicate.
 *
 * New position is the same as source one (with duplicated pieces list),
 * except:
 * - moves list is empty
 * - bestmove is NULL
 * - nodecount is set to zero
 * - eval is set to EVAL_INVALID
 * - moves_generated ans moves_counted are unset
 * - check is set to zero
 *
 * @return: The new position.
 *
 * TODO: merge with pos_new - NULL for init, non null for duplicate
 */
pos_t *pos_dup(pos_t *pos)
{
    pos_t *newpos = safe_malloc(sizeof(pos_t));

    //board = new->board;
    *newpos = *pos;
    //new->bestmove = NULL;
    newpos->node_count = 0;
    //new->eval = EVAL_INVALID;
    //new->moves_generated = false;
    //new->moves_counted = false;
    //new->check[WHITE] = new->check[BLACK] = 0;
    return newpos;
}

/**
 * pos_del() - delete a position.
 * @pos: &position.
 */
void pos_del(pos_t *pos)
{
    safe_free(pos);
}

/**
 * pos_clear() - clear a position.
 * @pos: &position.
 */
pos_t *pos_clear(pos_t *pos)
{
    printf("size(pos_board=%lu elt=%lu\n", sizeof(pos->board), sizeof(int));
    //for (square square = A1; square <= H8; ++square)
    //    pos->board[square] = EMPTY;

    SET_WHITE(pos->turn);
    pos->node_count = 0;
    pos->turn = 0;
    pos->clock_50 = 0;
    pos->plycount = 0;
    pos->en_passant = SQUARE_NONE;
    pos->castle = 0;
    memset(pos->board, 0, sizeof(pos->board));
    //pos->curmove = 0;
    //pos->eval = 0;
    //pos->occupied[WHITE] = 0;
    //pos->occupied[BLACK] = 0;
    for (color_t color = WHITE; color <= BLACK; ++color) {
        for (piece_type_t piece = 0; piece <= KING; ++piece)
            pos->bb[color][piece] = 0;
        pos->controlled[WHITE] = 0;
        pos->controlled[BLACK] = 0;
    }
    //pos->mobility[WHITE] = 0;
    //pos->mobility[BLACK] = 0;
    //pos->moves_generated = false;
    //pos->moves_counted = false;
    /* remove pieces / moves */
    //pieces_del(pos, WHITE);
    //pieces_del(pos, BLACK);
    //moves_del(pos);

    return pos;
}

/**
 * pos_print() - Print position and fen on stdout.
 * @pos:  &position
 */
void pos_print(pos_t *pos)
{
    int rank, file;
    piece_t pc, *board = pos->board;
    char fen[92];

    //piece_list_t *wk = list_first_entry(&pos->pieces[WHITE], piece_list_t, list),
    //    *bk = list_first_entry(&pos->pieces[BLACK], piece_list_t, list);


    printf("  +---+---+---+---+---+---+---+---+\n");
    for (rank = 7; rank >= 0; --rank) {
        printf("%c |", rank + '1');
        for (file = 0; file < 8; ++file) {
            pc = board[sq_make(file, rank)];
            printf(" %s |", piece_to_sym_color(pc));
        }
        printf("\n  +---+---+---+---+---+---+---+---+\n");
    }
    printf("    A   B   C   D   E   F   G   H\n");
    printf("%s\n", pos2fen(pos, fen));
    //printf("Turn: %s.\n", IS_WHITE(pos->turn) ? "white" : "black");
    /*
     * printf("Kings: W:%c%c B:%c%c\n",
     *        FILE2C(F88(wk->square)),
     *        RANK2C(R88(wk->square)),
     *        FILE2C(F88(bk->square)),
     *        RANK2C(R88(bk->square)));
     */
    //printf("plies=%d clock50=%d\n", pos->plycount, pos->clock_50);
    //printf("Current move = %d\n", pos->curmove);
    //printf("Squares controlled: W:%d B:%d\n", popcount64(pos->controlled[WHITE]),
    //       popcount64(pos->controlled[BLACK]));
    //printf("Mobility: W:%u B:%u\n", pos->mobility[WHITE],
    //       pos->mobility[BLACK]);
}

/**
 * pos_pieces_print() - Print position pieces
 * @pos:  &position
 */
void pos_pieces_print(pos_t *pos)
{
    int bit, count, cur;
    char pname;
    u64 tmp;
    bitboard_t p;
    for (int color = WHITE; color <= BLACK; ++color) {
        for (int piece = KING; piece >= PAWN; --piece) {
            p = pos->bb[color][piece];
            count = popcount64(p);
            cur = 0;
            pname = piece_to_char(piece);
            printf("%c(0)%s", pname, count? ":": "");
            if (count) {
                bit_for_each64(bit, tmp, p) {
                    char cf = sq_file(bit), cr = sq_rank(bit);
                    printf("%s%c%c", cur? ",": "", FILE2C(cf), RANK2C(cr));
                    cur++;
                }

            }
            printf(" ");
        }
        printf("\n");
        //printf("White pieces (%d): \t", popcount64(pos->occupied[WHITE]));
        //piece_list_print(&pos->pieces[WHITE]);
        //printf("Black pieces (%d): \t", popcount64(pos->occupied[BLACK]));
        //piece_list_print(&pos->pieces[BLACK]);
    }
}


/*
inline void bitboard_print2_raw(bitboard_t bb1, bitboard_t bb2, char *title)
{
    int i;
    printf("%s%s", title? title: "", title? ":\n": "");

    printf("\tW: %#018lx\tB: %#018lx\n", bb1, bb2);
    for (i=56; i>=0; i-=8)
        printf("\t"BYTE_PRINT"\t\t"BYTE_PRINT"\n",
               BYTE2BIN(bb1>>i),
               BYTE2BIN(bb2>>i));
}
*/

/**
 * raw_board_print - print simple board (hex values)
 * @bb: the bitboard
 */
void raw_board_print(const pos_t *pos)
{

    for (rank_t r = RANK_8; r >= RANK_1; --r) {
        for (file_t f = FILE_A; f <= FILE_H; ++f)
            printf("%02o ", pos->board[sq_make(f, r)]);
        printf(" \n");
    }
    return;
}

/**
 * pos_bitboards_print() - Print position bitboards
 * @pos:  &position
 */
//void pos_bitboards_print(pos_t *pos)
//{
//    printf("Bitboards occupied :\n");
//    bitboard_print2(pos->occupied[WHITE], pos->occupied[BLACK]);
//    printf("Bitboards controlled :\n");
//    bitboard_print2(pos->controlled[WHITE], pos->controlled[BLACK]);
//
//}

/**
 * pos_check() - extensive position consistenci check.
 * @pos:  &position
 */
/*
 * void pos_check(position *pos)
 * {
 *     int rank, file;
 *     piece_t piece;
 *     board_t *board = pos->board;
 *
 *     /\* check that board and bitboard reflect same information *\/
 *     for (rank = 7; rank >= 0; --rank) {
 *         for (file = 0; file < 8; ++file) {
 *             piece_list_t *ppiece;
 *             printf("checking %c%c ", file+'a', rank+'1');
 *
 *             piece = board[SQ88(file, rank)].piece;
 *             ppiece= board[SQ88(file, rank)].s_piece;
 *             printf("piece=%s ", P_CSYM(piece));
 *             if (ppiece)
 *                 printf("ppiece=%s/sq=%#x ", P_CSYM(ppiece->piece), ppiece->square);
 *             switch(PIECE(piece)) {
 *                 case PAWN:
 *                     printf("pawn" );
 *                     break;
 *                 case KNIGHT:
 *                     printf("knight ");
 *                     break;
 *                 case BISHOP:
 *                     printf("bishop ");
 *                     break;
 *                 case ROOK:
 *                     printf("rook ");
 *                     break;
 *                 case QUEEN:
 *                     printf("queen ");
 *                     break;
 *                 case KING:
 *                     printf("king ");
 *                     break;
 *             }
 *             printf("\n");
 *         }
 *     }
 * }
 */

/*
 * void pos_del(pos_t *pos)
 * {
 *     pieces_del(pos, WHITE);
 *     pieces_del(pos, BLACK);
 *     moves_del(pos);
 *     pool_add(pos_pool, pos);
 * }
 */
