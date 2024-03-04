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
#include "board.h"
#include "attack.h"

/**
 * pos_new() - allocate a new position
 *
 * Position is not initialized.
 * If BUG_ON compilation is defined, the program will fail.
 *
 * @Return: The new position or NULL.
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
 * @Return: The new position.
 *
 * TODO: merge with pos_new - NULL for init, non null for duplicate
 */
pos_t *pos_dup(const pos_t *pos)
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
#   ifdef DEBUG_POS
    printf("size(pos_board=%lu elt=%lu\n", sizeof(pos->board), sizeof(int));
#   endif
    pos->node_count = 0;
    pos->turn = WHITE;
    pos->clock_50 = 0;
    pos->plycount = 0;
    pos->en_passant = SQUARE_NONE;
    pos->castle = 0;

    for (color_t color = WHITE; color <= BLACK; ++color) {
        for (piece_type_t piece = 0; piece <= KING; ++piece)
            pos->bb[color][piece] = 0;
        pos->controlled[color] = 0;
        pos->king[color] = SQUARE_NONE;
    }

    for (square_t sq = A1; sq <= H8; ++sq)
        pos->board[sq] = EMPTY;
    pos->moves.curmove = 0;
    pos->moves.nmoves = 0;

    return pos;
}

/**
 * pos_checkers() - find all checkers on a king.
 * @pos:   &position
 * @color: king color
 *
 * Get a bitboard of all checkers on @color king.
 * Just a wrapper over @sq_attackers().
 *
 * @return: a bitboard of attackers.
 */
bitboard_t pos_checkers(const pos_t *pos, const color_t color)
{
    return sq_attackers(pos, pos->king[color], OPPONENT(color));
}

/**
 * pos_checkers2str() - convert checkers to string.
 * @pos: &position
 * @str: destination string (should be at least 2*3 + 1 = 7 length)
 *
 * @return: The string.
 */
char *pos_checkers2str(const pos_t *pos, char *str)
{
    int sq, tmp;
    char *p = str;
    bit_for_each64(sq, tmp, pos->checkers) {
        const char *sqstr = sq_to_string(sq);
        *p++ = sqstr[0];
        *p++ = sqstr[1];
        *p++ = ' ';
    }
    *p = 0;
    return str;
}

/**
 * pos_check() - extensive position consistenci check.
 * @pos:    &position
 * @strict: if not zero, call bug_on() on any error.
 *
 * Check (hopefully) if position is valid:
 * - pawns on first or 8th rank
 * - number of pawns per color > 8
 * - total number of pieces per color > 16 or zero
 * - number of kings per color != 1
 * - discrepancy between bitboards per piece and ALL_PIECES per color
 * - discrepancy between bitboards and board
 * - side-to-move already checking opponent king.
 *
 * In case of errors, and @abort is true, @bug_on() is called, and program will
 * be terminated.
 * This function should be called with @abort == 0 during initialization phase
 * (eg after fen parsing), and with @abort != 0 otherwise (as we have some data
 * corruption).
 *
 * TODO: add more checks
 * - en-prise king for side to move.
 *
 * @Return: 0 if no error detected
 *          the number of detected error if @abort == 0.
 *          this function does not return if @abort != 0 and errors are found.
 */
int pos_check(const pos_t *pos, const int fatal)
{
    int n, count = 0, bbcount = 0, error = 0;

    /* pawns on 1st ot 8th rank */
    n = popcount64((pos->bb[WHITE][PAWN] | pos->bb[BLACK][PAWN]) &
                   (RANK_1bb | RANK_8bb));
    error += warn_on(n != 0);

    for (color_t color = WHITE; color <= BLACK; ++color) {
        /* pawn count */
        n = popcount64(pos->bb[color][PAWN]);
        error += warn_on(n > 8);
        /* king count */
        n = popcount64(pos->bb[color][KING]);
        error += warn_on(n != 1);
        /* pieces count */
        n = popcount64(pos->bb[color][ALL_PIECES]);
        error += warn_on(n == 0 || n > 16);
        bbcount += n;
    }
    for (square_t sq = 0; sq < 64; ++sq) {
        piece_t piece = pos->board[sq];
        bitboard_t match;
        if (piece == EMPTY)
            continue;
        color_t c = COLOR(piece);
        piece_type_t p = PIECE(piece);
        match = pos->bb[c][p] & mask(sq);
        error += warn_on(!match);
        count++;
    }
    /* occupied occupation is different from bitboards */
    error += warn_on(count != bbcount);
    /* is color to play in check ? */
    error += warn_on(pos_checkers(pos, OPPONENT(pos->turn)));

    bug_on(fatal && error);
    return error;
}

/**
 * pos_print() - Print position and fen on stdout.
 * @pos:  &position
 */
void pos_print(const pos_t *pos)
{
    char str[92];

    board_print(pos->board);
    printf("fen %s\n", pos2fen(pos, str));
    printf("checkers %s\n", pos_checkers2str(pos, str));
}

/**
 * pos_print_mask() - Print position and fen on stdout, with highlighted squares.
 * @pos:  &position
 * @mask: mask of highlighted squares.
 */
void pos_print_mask(const pos_t *pos, const bitboard_t mask)
{
    char fen[92];

    board_print_mask(pos->board, mask);
    printf("fen %s\n", pos2fen(pos, fen));
}

/**
 * pos_print_board_raw - print simple position board (octal/FEN symbol values)
 * @bb: the bitboard
 * @type: int, 0 for octal, 1 for fen symbol
 */
void pos_print_raw(const pos_t *pos, const int type)
{
    board_print_raw(pos->board, type);
    return;
}

/**
 * pos_print_pieces() - Print position pieces
 * @pos:  &position
 */
void pos_print_pieces(const pos_t *pos)
{
    int bit, count, cur;
    char *pname;
    u64 tmp;
    bitboard_t p;
    for (int color = WHITE; color <= BLACK; ++color) {
        for (int piece = KING; piece >= PAWN; --piece) {
            p = pos->bb[color][piece];
            count = popcount64(p);
            cur = 0;
            pname = piece_to_char(piece);
            printf("%s(0)%s", pname, count? ":": "");
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
    }
}
