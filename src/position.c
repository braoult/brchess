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

#include <brlib.h>
#include <bitops.h>

#include "chessdefs.h"
#include "position.h"
#include "bitboard.h"
#include "hq.h"
#include "fen.h"
#include "piece.h"
#include "misc.h"
#include "board.h"
#include "attack.h"
#include "hist.h"

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
 * Return a copy, allocated with malloc(1), of @pos.
 *
 * @Return: The new position.
 *
 * TODO: merge with pos_new - NULL for init, non null for duplicate
 */
pos_t *pos_dup(const pos_t *pos)
{
    pos_t *newpos = safe_malloc(sizeof(pos_t));

    *newpos = *pos;
    return newpos;
}

/**
 * pos_copy() - copy a position into another one.
 * @from: &position to duplicate.
 * @to: &destination position.
 *
 * Return a copy of @from into @to.
 *
 * @Return: @to.
 */
pos_t *pos_copy(const pos_t *from, pos_t *to)
{
    *to = *from;
    return to;
}

/**
 * pos_del() - delete a position.
 * @pos: &position.
 *
 */
void pos_del(pos_t *pos)
{
    safe_free(pos);
}

/**
 * pos_clear() - clear a position.
 * @pos: &position.
 *
 * @return: @pos.
 */
pos_t *pos_clear(pos_t *pos)
{
#   ifdef DEBUG_POS
    printf("size(pos_board=%lu elt=%lu\n", sizeof(pos->board), sizeof(int));
#   endif
    pos->node_count = 0;
    pos->turn = WHITE;

    /* move_do/undo position state */
    pos->key = 0;
    pos->en_passant = SQUARE_NONE;
    pos->castle = 0;
    pos->clock_50 = 0;
    pos->plycount = 0;
    pos->move = MOVE_NONE;
    pos->captured = NO_PIECE;

    for (square_t sq = A1; sq <= H8; ++sq)
        pos->board[sq] = EMPTY;

    for (color_t color = WHITE; color <= BLACK; ++color) {
        for (piece_type_t piece = 0; piece <= KING; ++piece)
            pos->bb[color][piece] = 0;
        //pos->controlled[color] = 0;
        pos->king[color] = SQUARE_NONE;
    }

    pos->checkers = 0;
    pos->pinners = 0;
    pos->blockers = 0;

    return pos;
}

/**
 * pos_cmp() - compare two positions..
 * @pos1, @pos2: The two &position.
 *
 * Used only for move_{do,undo} test/debug.
 *
 * @return: true if equal, false otherwise.
 */
bool pos_cmp(const pos_t *pos1, const pos_t *pos2)
{
#define _cmpf(a) (pos1->a != pos2->a)
    bool ret = false;

    if (_cmpf(node_count) || _cmpf(turn))
        goto end;

    /* move_do/undo position state */
    if (_cmpf(key) || _cmpf(en_passant) || _cmpf(castle) ||
        _cmpf(clock_50) || _cmpf(plycount) || _cmpf(captured))
        goto end;

    if (_cmpf(checkers) || _cmpf(pinners) || _cmpf(blockers))
        goto end;

    for (square_t sq = A1; sq <= H8; ++sq)
        if (_cmpf(board[sq]))
            goto end;

    for (color_t color = WHITE; color <= BLACK; ++color) {
        for (piece_type_t piece = ALL_PIECES; piece <= KING; ++piece)
            if (_cmpf(bb[color][piece]))
                goto end;
        if (_cmpf(king[color]))
            goto end;
    }

    if (_cmpf(checkers) ||_cmpf(pinners) || _cmpf(blockers))
        goto end;

    /*
     * if (_cmpf(repeat.moves) ||
     *     memcmp(pos1->repeat.key, pos2->repeat.key,
     *            pos1->repeat.moves * sizeof pos1->repeat.key))
     *     goto end;
     */

    ret = true;
end:
    return ret;
#undef _cmpf
}

/**
 * pos_checkers() - find all checkers on a king.
 * @pos:   &position
 * @color: king color
 *
 * Get a bitboard of all checkers on @color king.
 * Just a wrapper over @sq_attackers().
 *
 * @return: a bitboard of checkers.
 */
bitboard_t pos_checkers(const pos_t *pos, const color_t color)
{
    bitboard_t occ = pos_occ(pos);
    return sq_attackers(pos, occ, pos->king[color], OPPONENT(color));
}

/**
 * pos_set_checkers_pinners_blockers() - calculate checkers, pinners and blockers.
 * @pos:   &position
 *
 * Set position checkers, pinners and blockers on player-to-play king.
 * It should be slightly faster than @pos_checkers + @pos_set_pinners_blockers, as
 * some calculation will be done once.
 */
void pos_set_checkers_pinners_blockers(pos_t *pos)
{
    int us = pos->turn, them = OPPONENT(us);
    bitboard_t occ = pos_occ(pos);
    bitboard_t attackers;
    bitboard_t checkers = 0, blockers = 0, pinners = 0;
    bitboard_t targets, tmpcheckers, maybeblockers, tmppinners;
    square_t king = pos->king[us];
    int pinner;

    /* bishop type - we attack with a bishop from king position */
    attackers = pos->bb[them][BISHOP] | pos->bb[them][QUEEN];

    /* targets is all "target" pieces if K was a bishop */
    targets = hq_bishop_moves(occ, king) & occ;

    /* checkers = only opponent B/Q */
    tmpcheckers = targets & attackers;
    checkers |= tmpcheckers;

    /* maybe blockers = we remove checkers, to look "behind" */
    maybeblockers = targets & ~tmpcheckers;

    /* we find second targets, by removing first ones (excl. checkers) */
    if (maybeblockers) {
        targets = hq_bishop_moves(occ ^ maybeblockers, king) ^ tmpcheckers;

        /* pinners = only B/Q */
        tmppinners = targets & attackers;

        /* blockers = we find occupied squares between pinner and king */
        while (tmppinners) {
            pinner = bb_next(&tmppinners);
            pinners |= BIT(pinner);
            blockers |= bb_between[pinner][king] & maybeblockers;
        }
    }

    /* same for rook type */
    attackers = pos->bb[them][ROOK] | pos->bb[them][QUEEN];
    targets = hq_rook_moves(occ, king) & occ;

    tmpcheckers = targets & attackers;
    checkers |= tmpcheckers;

    maybeblockers = targets & ~tmpcheckers;
    if (maybeblockers) {
        targets = hq_rook_moves(occ ^ maybeblockers, king) ^ tmpcheckers;
        tmppinners = targets & attackers;
        while (tmppinners) {
            pinner = bb_next(&tmppinners);
            pinners |= BIT(pinner);
            blockers |= bb_between[pinner][king] & maybeblockers;
        }
    }

    /* pawns & knights */
    checkers |= bb_pawn_attacks[us][king] & pos->bb[them][PAWN];
    checkers |= bb_knight[king] & pos->bb[them][KNIGHT];

    pos->checkers = checkers;
    pos->pinners = pinners;
    pos->blockers = blockers;
}

/**
 * pos_set_pinners_blockers() - set position pinners and blockers.
 * @pos:   &position
 *
 * set position pinners and blockers on player-to-play king.
 */
void pos_set_pinners_blockers(pos_t *pos)
{
    color_t color = pos->turn;
    square_t king = pos->king[color];
    bitboard_t tmp, occ = pos_occ(pos), blockers = 0;
    int pinner;

    pos->pinners = sq_pinners(pos, king, OPPONENT(pos->turn));
    bit_for_each64(pinner, tmp, pos->pinners) {
        //bitboard_t blocker =
        // warn_on(popcount64(blocker) != 1);
        blockers |= bb_between_excl[pinner][king] & occ;
    }
    pos->blockers = blockers;
    return;
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
        //warn_on(popcount64(blocker) != 1);
        //if (popcount64(blocker) != 1) {
        //    printf("n blockers = %d\n", popcount64(blocker));
        //    bb_print("blockers", blocker);
        //}
        blockers |= bb_between_excl[pinner][king] & occ;
    }
    return blockers;
}

/**
 * pos_ok() - extensive position consistency check.
 * @pos:    &position
 * @strict: if true, call bug_on() on any error.
 *
 * Perform some validity check on position @pos:
 * - pawns on 1st or 8th rank
 * - number of pawns > 8 (per color)
 * - total number of pieces > 16 or zero (per color)
 * - number of kings != 1 (per color)
 * - discrepancy between board and king (per color)
 * - discrepancy between bitboards and board (per color)
 * - side-to-move already checking opponent king
 * - side-to-move in check more than twice
 * - kings distance is 1
 * - TODO: - castling / e.p. flags
 *
 * In case of errors, and @strict is true, @bug_on() is called, and program will
 * be terminated.
 * This function should be called with @strict == false during initialization phase
 * (eg after fen parsing), and with @strict == true otherwise (as we have some data
 * corruption).
 *
 * @return: (if @strict is false) return true if check is ok, false otherwise.
 */
bool pos_ok(pos_t *pos, const bool strict)
{
    int n, count = 0, bbcount = 0, error = 0;
    color_t __unused us = pos->turn, __unused them = OPPONENT(us);

    /* force BUG_ON and WARN_ON */
#   pragma push_macro("BUG_ON")
#   pragma push_macro("WARN_ON")
#   undef BUG_ON
#   define BUG_ON
#   undef WARN_ON
#   define WARN_ON
#   include <bug.h>

    /* pawns on 1st ot 8th rank */
    error += warn_on((pos->bb[WHITE][PAWN] | pos->bb[BLACK][PAWN]) &
                     (RANK_1bb | RANK_8bb));

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
        __unused bitboard_t match;
        if (piece == EMPTY)
            continue;
        color_t c = COLOR(piece);
        piece_type_t p = PIECE(piece);
        match = pos->bb[c][p] & BIT(sq);
        error += warn_on(!match);
        count++;
    }
    /* occupied board is different from bitboards */
    error += warn_on(count != bbcount);
    /* is opponent already in check ? */
    error += warn_on(pos_checkers(pos, them));
    /* is color to play in check more than twice ? */
    error += warn_on(popcount64(pos_checkers(pos, us)) > 2);
    /* kings distance is less than 2 */
    error += warn_on(sq_dist(pos->king[WHITE], pos->king[BLACK]) < 2);
    /* e.p. and castling rights check */
    error += fen_ok(pos, false);

    if (strict) {
        bug_on(error);
        /* not reached */
    }
    return error? false: true;
#   pragma pop_macro("WARN_ON")
#   pragma pop_macro("BUG_ON")
}

/**
 * pos_print() - Print position and fen on stdout.
 * @pos:  &position
 */
void pos_print(const pos_t *pos)
{
    char str[128];

    board_print(pos->board);
    printf("fen: %s\n", pos2fen(pos, str));
    printf("last move:%s ", move_to_str(str, pos->move, 0));
    printf("key:%lx\n", pos->key);
    printf("checkers:%s ", pos_checkers2str(pos, str, sizeof(str)));
    printf("pinners: %s ", pos_pinners2str(pos, str, sizeof(str)));
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
 * pos_print_raw - print simple position board (octal/FEN symbol values)
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
