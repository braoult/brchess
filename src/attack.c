/* attack.c - attack functions.
 *
 * Copyright (C) 2024 Bruno Raoult ("br")
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
#include <stdarg.h>

#include "chessdefs.h"
#include "bitboard.h"
#include "position.h"
#include "hyperbola-quintessence.h"
#include "attack.h"


/**
 * sq_is_attacked() - find if a square is attacked
 * @pos: position
 * @occ: occupation mask used
 * @sq:  square to test
 * @c:   attacker color
 *
 * Find if a @c piece attacks @sq.
 *
 * Algorithm:  We perform a reverse attack, and check if any given
 * piece type on @sq can attack a @c piece of same type.
 *
 * For example, if @c is white, we test for all T in P,N,B,R,Q,K if
 * T on @sq could attack a white T piece.
 *
 * @Return: true if @sq is attacked by a @c piece, false otherwise.
 */
bool sq_is_attacked(const pos_t *pos, const bitboard_t occ, const square_t sq, const color_t c)
{
    bitboard_t sqbb = mask(sq);
    color_t opp = OPPONENT(c);


    //pos_print_raw(pos, 1);

    /* bishop / queen */
    if (hyperbola_bishop_moves(occ, sq) & (pos->bb[c][BISHOP] | pos->bb[c][QUEEN]))
        return true;

    /* rook / queen */
    if (hyperbola_rook_moves(occ, sq) & (pos->bb[c][ROOK] | pos->bb[c][QUEEN]))
        return true;

    /* pawn */
    if ((pawn_shift_upleft(sqbb, opp) | pawn_shift_upright(sqbb, opp)) & pos->bb[c][PAWN])
        return true;

    /* knight */
    if (bb_knight_moves(pos->bb[c][KNIGHT], sq))
        return true;

    /* king */
    if (bb_king_moves(pos->bb[c][KING], sq))
        return true;

    return false;
}

/**
 * is_in_check() - find if a king is in check.
 * @pos:   position
 * @color: king color
 *
 * Find if a @c king is in check. This function is a wrapper over @sq_is_attacked().
 *
 * @Return: true if @sq is attacked by a @c piece, false otherwise.
 */
bool is_in_check(const pos_t *pos, const color_t color)
{
    bitboard_t occ = pos_occ(pos);
    return sq_is_attacked(pos, occ, pos->king[color], OPPONENT(color));
}

/**
 * sq_attackers() - find attackers on a square
 * @pos: position
 * @occ: occupation mask used
 * @sq:  square to test
 * @c:   attacker color
 *
 * Find all @c attacks on @sq. En-passant is not considered.
 *
 * Algorithm:  We perform a reverse attack, and check if any given
 * piece type on @sq can attack a @c piece of same type.
 *
 * For example, if @c is white, we test for all T in P,N,B,R,Q,K if
 * T on @sq could attack a white T piece.
 *
 * @Return: a bitboard of attackers.
 */
bitboard_t sq_attackers(const pos_t *pos, const bitboard_t occ, const square_t sq, const color_t c)
{
    bitboard_t attackers = 0, tmp;
    bitboard_t sqbb = mask(sq);
    //bitboard_t occ = pos_occ(pos);
    bitboard_t to;
    color_t opp = OPPONENT(c);

    /* pawn */
    to =  pos->bb[c][PAWN];
    tmp = pawn_shift_upleft(sqbb, opp) & to;
    attackers |= tmp;
#   ifdef DEBUG_ATTACK_ATTACKERS1
    bb_print("att pawn upleft", tmp);
#   endif
    tmp = pawn_shift_upright(sqbb, opp) & to;
    attackers |= tmp;
#   ifdef DEBUG_ATTACK_ATTACKERS1
    bb_print("att pawn upright", tmp);
#   endif

    /* knight & king */
    to = pos->bb[c][KNIGHT];
    tmp = bb_knight_moves(to, sq);
    attackers |= tmp;
#   ifdef DEBUG_ATTACK_ATTACKERS1
    bb_print("att knight", tmp);
#   endif
    to = pos->bb[c][KING];
    tmp = bb_king_moves(to, sq);
    attackers |= tmp;
#   ifdef DEBUG_ATTACK_ATTACKERS1
    bb_print("att king", tmp);
#   endif

    /* bishop / queen */
    to = pos->bb[c][BISHOP] | pos->bb[c][QUEEN];
    tmp = hyperbola_bishop_moves(occ, sq) & to;
    attackers |= tmp;
#   ifdef DEBUG_ATTACK_ATTACKERS1
    bb_print("att bishop/queen", tmp);
#   endif

    /* rook / queen */
    to = pos->bb[c][ROOK] | pos->bb[c][QUEEN];
    tmp = hyperbola_rook_moves(occ, sq) & to;
    attackers |= tmp;
#   ifdef DEBUG_ATTACK_ATTACKERS1
    bb_print("att rook/queen", tmp);
    bb_print("ATTACKERS", attackers);
    printf("attackers=%lx\n", attackers);
#   endif

    return attackers;
}

/**
 * sq_pinners() - get "pinners" on a square
 * @pos: position
 * @sq:  square to test
 * @color:   attacker color
 *
 * Find all @c pieces which are separated from @sq by only one piece (of
 * any color).
 *
 * @Return: bitboard of pinners.
 */
bitboard_t sq_pinners(const pos_t *pos, const square_t sq, const color_t color)
{
    bitboard_t attackers, pinners = 0;
    bitboard_t occ = pos_occ(pos);
    bitboard_t maybe_pinner, tmp, lines;

    /* bishop type */
    attackers = pos->bb[color][BISHOP] | pos->bb[color][QUEEN];
    /* occupancy on sq diag and antidiag */
    lines = (bb_sqdiag[sq] | bb_sqanti[sq]) & occ;
    bit_for_each64(maybe_pinner, tmp, attackers) {
        bitboard_t between = bb_between_excl[maybe_pinner][sq];
        /* keep only squares between AND on sq diag/anti */
        if (popcount64(between & lines) == 1)
            pinners |= mask(maybe_pinner);
    }

    /* same for rook type */
    attackers = pos->bb[color][ROOK] | pos->bb[color][QUEEN];
    lines = (bb_sqrank[sq] | bb_sqfile[sq]) & occ;
    bit_for_each64(maybe_pinner, tmp, attackers) {
        bitboard_t between = bb_between_excl[maybe_pinner][sq];
        if (popcount64(between & lines) == 1)
            pinners |= mask(maybe_pinner);
    }
#   ifdef DEBUG_ATTACK_ATTACKERS1
    char str[32];
    printf("pinners : %s\n", pos_pinners2str(pos, str, sizeof(str)));
    printf("pinners : %lx\n", pinners);
#   endif
    return pinners;
}

/**
 * sq_attackers() - find all attackers on a square
 * @pos: position
 * @sq:  square to test
 *
 * Find all attacks on @sq. En-passant is not considered.
 * Just a wrapper over @sq_attackers().
 *
 * @Return: a bitboard of attackers.
 */
bitboard_t sq_attackers_all(const pos_t *pos, const square_t sq)
{
    bitboard_t occ = pos_occ(pos);
    return sq_attackers(pos, occ, sq, WHITE) | sq_attackers(pos, occ, sq, BLACK);
}
