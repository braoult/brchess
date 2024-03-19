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

    pos->en_passant = SQUARE_NONE;
    pos->castle = 0;
    pos->clock_50 = 0;
    pos->plycount = 0;

    for (square_t sq = A1; sq <= H8; ++sq)
        pos->board[sq] = EMPTY;

    for (color_t color = WHITE; color <= BLACK; ++color) {
        for (piece_type_t piece = 0; piece <= KING; ++piece)
            pos->bb[color][piece] = 0;
        pos->controlled[color] = 0;
        pos->king[color] = SQUARE_NONE;
    }

    pos->checkers = 0;
    pos->pinners = 0;
    pos->blockers = 0;

    //pos->moves.curmove = 0;
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
    bitboard_t occ = pos_occ(pos);
    return sq_attackers(pos, occ, pos->king[color], OPPONENT(color));
}

/**
 * pos_king_pinners() - get the "pinners" on a king "pinners".
 * @pos:   &position
 * @color: king color.
 *
 * get position "pinners" on @color king. Here, pinner means a piece separated from king
 * by one piece (any color) only.
 * This is just a wrapper over @sq_pinners().
 *
 * @return: pinners bitboard.
 */
bitboard_t pos_king_pinners(const pos_t *pos, const color_t color)
{
    return sq_pinners(pos, pos->king[color], OPPONENT(pos->turn));
}

/**
 * pos_king_blockers() - get the pin blockers on a king.
 * @pos:   &position
 * @color: king color.
 * @pinners: pinners bitboard.
 *
 * get @pinners blockers pieces on @color king.
 *
 * @return: blockers bitboard.
 */
bitboard_t pos_king_blockers(const pos_t *pos, const color_t color, const bitboard_t pinners)
{
    bitboard_t tmp, blockers = 0, occ = pos_occ(pos);
    square_t pinner, king = pos->king[color];

    bit_for_each64(pinner, tmp, pinners) {
        bitboard_t blocker = bb_between_excl[pinner][king] & occ;
        warn_on(popcount64(blocker) != 1);
        if (popcount64(blocker) != 1) {
            printf("n blockers = %d\n", popcount64(blocker));
            bb_print("blockers", blocker);
        }
        blockers |= blocker;
    }
    return blockers;
}

/**
 * pos_check() - extensive position consistency check.
 * @pos:    &position
 * @strict: if true, call bug_on() on any error.
 *
 * Perform some validity check on position @pos:
 * - pawns on 1st or 8th rank
 * - number of pawns > 8 (per color)
 * - total number of pieces > 16 or zero (per color)
 * - number of kings != 1 (per color)
 * - discrepancy between board and king (per color)
 * - discrepancy between piece bitboards and ALL_PIECES bitboards (per color)
 * - discrepancy between bitboards and board (per color)
 * - side-to-move already checking opponent king
 * - side-to-move in check more than twice
 * - kings distance is 1
 *
 * In case of errors, and @strict is true, @bug_on() is called, and program will
 * be terminated.
 * This function should be called with @strict == false during initialization phase
 * (eg after fen parsing), and with @strict == true otherwise (as we have some data
 * corruption).
 *
 * TODO: add more checks:
 * - kings attacking each other
 *
 * @Return: Number of detected error (only if @strict is false).
 */
int pos_check(const pos_t *pos, const bool strict)
{
    int n, count = 0, bbcount = 0, error = 0;
    bitboard_t tmp;

    /* pawns on 1st ot 8th rank */
    tmp = (pos->bb[WHITE][PAWN] | pos->bb[BLACK][PAWN]) & (RANK_1bb | RANK_8bb);
    error += warn_on(tmp);

    for (color_t color = WHITE; color <= BLACK; ++color) {
        /* pawn count */
        n = popcount64(pos->bb[color][PAWN]);
        error += warn_on(n > 8);
        /* king count */
        n = popcount64(pos->bb[color][KING]);
        error += warn_on(n != 1);
        /* king mismatch with board */
        error += warn_on(PIECE(pos->board[pos->king[color]]) != KING);
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
    /* is opponent already in check ? */
    error += warn_on(pos_checkers(pos, OPPONENT(pos->turn)));
    /* is color to play in check more than twice ? */
    error += warn_on(popcount64(pos_checkers(pos, OPPONENT(pos->turn))) > 2);
    /* kings distance is less than 2 */
    error += warn_on(sq_dist(pos->king[WHITE], pos->king[BLACK]) < 2);

    bug_on(strict && error);
    return error;
}

/**
 * pos_print() - Print position and fen on stdout.
 * @pos:  &position
 */
void pos_print(const pos_t *pos)
{
    char str[128];

    board_print(pos->board);
    printf("fen %s\n", pos2fen(pos, str));
    printf("checkers: %s\n", pos_checkers2str(pos, str, sizeof(str)));
    printf("pinners : %s\n", pos_pinners2str(pos, str, sizeof(str)));
    printf("blockers: %s\n", pos_blockers2str(pos, str, sizeof(str)));
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
    int sq, count, cur;
    char *pname;
    u64 tmp;
    bitboard_t p;
    for (int color = WHITE; color <= BLACK; ++color) {
        for (int piece = KING; piece >= PAWN; --piece) {
            p = pos->bb[color][piece];
            count = popcount64(p);
            cur = 0;
            pname = piece_to_char(p);
            printf("%s(%d)%s", pname, count, count? ":": "");
            if (count) {
                bit_for_each64(sq, tmp, p) {
                    // char cf = sq_file(bit), cr = sq_rank(bit);
                    printf("%s%s", cur? ",": "", sq_to_string(sq));
                    cur++;
                }
            }
            printf(" ");
        }
        printf("\n");
    }
}
