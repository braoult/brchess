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

/**
 * square_attackers() - find attackers on a square
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
bitboard_t sq_attackers(pos_t *pos, square_t sq, color_t c)
{
    bitboard_t attackers = 0;
    bitboard_t from = mask(sq);
    bitboard_t c_pieces = pos->bb[c][ALL_PIECES];
    bitboard_t occ = c_pieces | pos->bb[OPPONENT(c)][ALL_PIECES];
    bitboard_t to;

    /* pawn */
    to =  pos->bb[c][PAWN];
    attackers |= pawn_push_upleft(from, c) & to;
    attackers |= pawn_push_upright(from, c) & to;

    /* knight & king */
    to = pos->bb[c][KNIGHT];
    attackers |= bb_knight_moves(c_pieces, from);
    to = pos->bb[c][KING];
    attackers |= bb_king_moves(c_pieces, from);

    /* bishop / queen */
    to = pos->bb[c][BISHOP] | pos->bb[c][QUEEN];
    attackers |= hyperbola_bishop_moves(occ, from) & to;

    /* rook / queen */
    to = pos->bb[c][ROOK] | pos->bb[c][QUEEN];
    attackers |= hyperbola_rook_moves(occ, from) & to;

    return attackers;
}
