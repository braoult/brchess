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
 * sq_attackers() - find attackers on a square
 * @pos: position
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
 *
 */
bitboard_t sq_attackers(const pos_t *pos, const square_t sq, const color_t c)
{
    bitboard_t attackers = 0;
    bitboard_t sqbb = mask(sq);
    bitboard_t c_pieces = pos->bb[c][ALL_PIECES];
    bitboard_t occ = c_pieces | pos->bb[OPPONENT(c)][ALL_PIECES];
    bitboard_t to;
    color_t opp = OPPONENT(c);

    /* pawn */
    to =  pos->bb[c][PAWN];
    attackers |= pawn_shift_upleft(sqbb, opp) & to;
#   ifdef DEBUG_ATTACK
    bitboard_print("sq_attackers after pawn upleft", attackers);
#   endif
    attackers |= pawn_shift_upright(sqbb, opp) & to;
#   ifdef DEBUG_ATTACK
    bitboard_print("sq_attackers pawn upright", attackers);
#   endif

    /* knight & king */
    to = pos->bb[c][KNIGHT];
    attackers |= bb_knight_moves(to, sq);
#   ifdef DEBUG_ATTACK
    bitboard_print("sq_attackers after knight", attackers);
#   endif
    to = pos->bb[c][KING];
    attackers |= bb_king_moves(to, sq);
#   ifdef DEBUG_ATTACK
    bitboard_print("sq_attackers after king", attackers);
#   endif

    /* bishop / queen */
    to = pos->bb[c][BISHOP] | pos->bb[c][QUEEN];
    attackers |= hyperbola_bishop_moves(occ, sq) & to;
#   ifdef DEBUG_ATTACK
    bitboard_print("sq_attackers after bishop/queen", attackers);
#   endif

    /* rook / queen */
    to = pos->bb[c][ROOK] | pos->bb[c][QUEEN];
#   ifdef DEBUG_ATTACK
    bitboard_print("sq_attackers after queen", attackers);
#   endif
    attackers |= hyperbola_rook_moves(occ, sq) & to;
#   ifdef DEBUG_ATTACK
    bitboard_print("sq_attackers after rook/queen", attackers);
#   endif

    return attackers;
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
    return sq_attackers(pos, sq, WHITE) | sq_attackers(pos, sq, BLACK);
}
