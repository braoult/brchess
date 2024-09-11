/* move-do.c - move do/undo.
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

#include <malloc.h>
#include <ctype.h>
#include <stdlib.h>

#include <brlib.h>
#include <likely.h>
#include <bug.h>

#include "chessdefs.h"
#include "move.h"
#include "position.h"
#include "move-do.h"
#include "hash.h"

/**
 * sq_castle - castling rights per square.
 *
 * position castling rights mask has to be AND'ed with this array 'from' and 'to'.
 */
castle_rights_t sq_castle[64] = {
    [A1] = ~CASTLE_WQ, [B1 ... D1] = CASTLE_ALL,
    [E1] = ~CASTLE_W,  [F1 ... G1] = CASTLE_ALL,
    [H1] = ~CASTLE_WK, [A2 ... H7] = CASTLE_ALL,
    [A8] = ~CASTLE_BQ, [B8 ... D8] = CASTLE_ALL,
    [E8] = ~CASTLE_B,  [F8 ... G8] = CASTLE_ALL,
    [H8] = ~CASTLE_BK,
};

/**
 * move_do() - do move.
 * @pos:    &pos_t position
 * @move:   move to apply
 * @state:  &state_t address where irreversible changes will be saved
 *
 * @move is applied to @pos:
 * - bitboards and board are updated
 * - counters are updated:
 *   - move count
 *   - 50-moves rule count
 * - flags are possibly updated:
 *   - castling
 *   - en-passant
 * - captured piece (excl. en-passant)
 * - tt hash values are updated for:
 *   - side-to-move
 *   - en-passant
 *   - castling rights.
 *
 * @return: updated pos.
 */
pos_t *move_do(pos_t *pos, const move_t move, state_t *state)
{
    color_t us = pos->turn, them = OPPONENT(us);
    square_t from = move_from(move), to = move_to(move);
    piece_t piece = pos->board[from];
    piece_t captured = pos->board[to];
    piece_type_t ptype = PIECE(piece);
    piece_t new_piece = piece;
    int up = sq_up(us);
    hkey_t key = pos->key;

    *state = pos->state;                          /* save irreversible changes */

    /* update key: switch turn, reset ep */
    key ^= zobrist_turn;
    key ^= zobrist_ep[EP_ZOBRIST_IDX(pos->en_passant)];

    ++pos->clock_50;
    ++pos->plycount;
    pos->en_passant = SQUARE_NONE;
    pos->turn = them;
    pos->captured = captured;
    pos->move = move;

    bug_on(COLOR(piece) != us);

    /* special moves: captures, pawn double push, en-passant, castling.
     * All these moves are exclusive (so we use if ... else)
     */
    if (captured != EMPTY) {                      /* capture: remove piece */
        pos->clock_50 = 0;
        bug_on(pos->board[to] == EMPTY || COLOR(pos->captured) != them);
        key ^= zobrist_pieces[captured][to];
        pos_clr_sq(pos, to);
    } else if (ptype == PAWN) {
        pos->clock_50 = 0;
        if (from + up + up == to) {               /* double push: set e.p. */
            square_t ep = from + up;
            /* check that opponent can e.p. */
            if (bb_pawn_attacks[us][ep] & pos->bb[them][PAWN]) {
                pos->en_passant = ep;
                key ^= zobrist_ep[EP_ZOBRIST_IDX(pos->en_passant)];
            }
        } else if (is_enpassant(move)) {          /* e.p.: clear grabbed pawn */
            square_t grabbed = to - up;
            piece_t pc = pos->board[grabbed];
            key ^= zobrist_pieces[pc][grabbed];
            pos_clr_sq(pos, grabbed);
        }
    } else if (is_castle(move)) {                 /* castling: handle rook move */
        square_t rookfrom, rookto;
        if (to > from) {                          /* o-o */
            rookfrom = to + 1;
            rookto = to - 1;
        } else {                                  /* o-o-o */
            rookfrom = to - 2;
            rookto = to + 1;
        }
        key ^= zobrist_pieces[pos->board[rookfrom]][rookto] ^
            zobrist_pieces[pos->board[rookfrom]][rookfrom];
        pos_set_sq(pos, rookto, pos->board[rookfrom]);
        pos_clr_sq(pos, rookfrom);
    }

    /* common part: captures and non captures
     * We could improve slightly, as en-passant and double push will make
     * the following tests.
     * Improvement could be to add promotion test and king test twice above.
     * (in capture and after ).
     */
    if (is_promotion(move)) {
        bug_on(sq_rank(to) != sq_rel_rank(RANK_8, us));
        new_piece = MAKE_PIECE(move_promoted(move), us);
    }

    key ^= zobrist_pieces[piece][from] ^ zobrist_pieces[new_piece][to];
    pos_clr_sq(pos, from);
    pos_set_sq(pos, to, new_piece);

    /* update castling flags */
    key ^= zobrist_castling[pos->castle];
    pos->castle &= sq_castle[from] & sq_castle[to];
    key ^= zobrist_castling[pos->castle];

    //if (ptype == KING) {
    //    pos->king[us] = to;
    //} else
    /* do this always, cheaper than test on K move */
    pos->king[us] = ctz64(pos->bb[us][KING]);

    pos->key = key;

    zobrist_verify(pos);

    return pos;
}

/**
 * move_undo() - undo move.
 * @pos:    &pos_t position
 * @move:   move to undo
 * @state:  &state_t address where irreversible changes were saved
 *
 * @move is applied to @pos:
 * - bitboards and board are updated
 * - previous information is restored:
 *   - castling
 *   - en-passant
 *   - captured piece (excl. en-passant)
 *   - move count
 *   - 50-moves rule count
 *
 * @return: pos.
 */
pos_t *move_undo(pos_t *pos, const move_t move, const state_t *state)
{
    color_t them = pos->turn, us = OPPONENT(them);
    square_t from = move_from(move), to = move_to(move);
    piece_t piece = pos->board[to];
    int up = sq_up(them);

    /* common part: captures and non captures
     * We could improve slightly, as en-passant and double push will make
     * the following tests.
     * Improvement could be to add promotion test and king test twice above.
     * (in capture and after ).
     */
    if (is_promotion(move))
        piece = MAKE_PIECE(PAWN, us);

    pos_clr_sq(pos, to);
    pos_set_sq(pos, from, piece);

    /* special moves: capture, king (+ castling), en-passant
     */
    if (pos->captured != EMPTY) {                 /* captured: restore piece */
        pos_set_sq(pos, to, pos->captured);
    } else if (is_castle(move)) {                 /* castle: rook move */
        square_t rookfrom, rookto;
        if (to > from) {                          /* o-o */
            rookfrom = to - 1;
            rookto = to + 1;
        } else {                                  /* o-o-o */
            rookfrom = to + 1;
            rookto = to - 2;
        }
        pos_set_sq(pos, rookto, pos->board[rookfrom]);
        pos_clr_sq(pos, rookfrom);
    } else if (is_enpassant(move)) {              /* e.p.: restore grabbed pawn */
        square_t grabbed = to + up;
        pos_set_sq(pos, grabbed, MAKE_PIECE(PAWN, them));
    }

    /* do this always, cheaper than test on K move */
    pos->king[us] = ctz64(pos->bb[us][KING]);
    //if (PIECE(piece) == KING)
    //    pos->king[us] = from;

    pos->state = *state;                          /* restore irreversible changes */
    pos->turn = us;
    return pos;
}

/**
 * move_{do,undo}_alt - alternative move_do/move_undo (to experiment)
 */
pos_t *move_do_alt(pos_t *pos, const move_t move, state_t *state)
{
    color_t us = pos->turn, them = OPPONENT(us);
    square_t from = move_from(move), to = move_to(move);
    piece_t piece = pos->board[from];
    piece_t captured = pos->board[to];
    piece_type_t ptype = PIECE(piece);
    piece_t new_piece = piece;
    int up = sq_up(us);
    hkey_t key = pos->key;

    *state = pos->state;                          /* save irreversible changes */

    /* update key: switch turn, reset castling and ep */
    key ^= zobrist_turn;
    key ^= zobrist_castling[pos->castle];
    key ^= zobrist_ep[EP_ZOBRIST_IDX(pos->en_passant)];

    ++pos->clock_50;
    ++pos->plycount;
    pos->en_passant = SQUARE_NONE;
    pos->turn = them;
    pos->captured = captured;
    pos->move = move;

    bug_on(COLOR(piece) != us);

    if (is_promotion(move)) {
        bug_on(sq_rank(to) != sq_rel_rank(RANK_8, us));
        new_piece = MAKE_PIECE(move_promoted(move), us);
    }

    if (captured != EMPTY) {
        pos->clock_50 = 0;
        //pos->captured = pos->board[to];           /* save capture info */
        bug_on(pos->board[to] == EMPTY || COLOR(pos->captured) != them);
        key ^= zobrist_pieces[captured][to];
        pos_clr_sq(pos, to);                      /* clear square */
    } else if (is_castle(move)) {                 /* handle rook move */
        square_t rookfrom, rookto;
        if (to > from) {
            rookfrom = sq_rel(H1, us);
            rookto = sq_rel(F1, us);
        } else {
            rookfrom = sq_rel(A1, us);
            rookto = sq_rel(D1, us);
        }
        key ^= zobrist_pieces[pos->board[rookfrom]][rookto] ^
            zobrist_pieces[pos->board[rookfrom]][rookfrom];
        pos_set_sq(pos, rookto, pos->board[rookfrom]);
        pos_clr_sq(pos, rookfrom);
        pos->castle = clr_castle(pos->castle, us);
    } else if (ptype == PAWN) {                   /* pawn non capture or e.p. */
        pos->clock_50 = 0;
        if (from + up + up == to) {               /* if pawn double push, set e.p. */
            square_t ep = from + up;
            if (bb_pawn_attacks[us][ep] & pos->bb[them][PAWN]) {
                pos->en_passant = ep;
                key ^= zobrist_ep[EP_ZOBRIST_IDX(pos->en_passant)];
            }
        } else if (is_enpassant(move)) {          /* clear grabbed pawn */
            square_t grabbed = to - up;
            piece_t pc = pos->board[grabbed];
            key ^= zobrist_pieces[pc][grabbed];
            pos_clr_sq(pos, grabbed);
        }
    }

    key ^= zobrist_pieces[piece][from] ^ zobrist_pieces[new_piece][to];
    pos_clr_sq(pos, from);
    pos_set_sq(pos, to, new_piece);

    if (ptype == KING)
        pos->king[us] = to;

    /* update castling flags
     * As we always consider flags are valid, we :
     * - adjust our flags if relative from is "E1", "A1", H1"
     * - adjust opp flags if relative to   if "A8", H8"
     */
    if (can_castle(pos->castle, us)) {            /* do we save time with this test ? */
        square_t rel_e1 = sq_rel(E1, us);
        square_t rel_a1 = sq_rel(A1, us);
        square_t rel_h1 = sq_rel(H1, us);
        if (from == rel_e1)
            pos->castle = clr_castle(pos->castle, us);
        else if (from == rel_a1)
            pos->castle = clr_ooo(pos->castle, us);
        else if (from == rel_h1)
            pos->castle = clr_oo(pos->castle, us);
    }
    if (can_castle(pos->castle, them)) {
        square_t rel_a8 = sq_rel(A8, us);
        square_t rel_h8 = sq_rel(H8, us);
        if (to == rel_a8)
            pos->castle = clr_ooo(pos->castle, them);
        else if (to == rel_h8)
            pos->castle = clr_oo(pos->castle, them);
    }

    /* update castling rights key */
    key ^= zobrist_castling[pos->castle];

    pos->key = key;

    zobrist_verify(pos);

    return pos;
}

pos_t *move_undo_alt(pos_t *pos, const move_t move, const state_t *state)
{
    color_t them = pos->turn, us = OPPONENT(them);
    square_t from = move_from(move), to = move_to(move);
    piece_t piece = pos->board[to];
    int up = sq_up(them);

    if (is_promotion(move))
        piece = MAKE_PIECE(PAWN, us);

    pos_clr_sq(pos, to);                          /* always clear "to" ... */
    pos_set_sq(pos, from, piece);                 /* ... and set "from" */

    if (PIECE(piece) == KING)
        pos->king[us] = from;

    if (pos->captured != EMPTY) {
        pos_set_sq(pos, to, pos->captured);       /* restore captured piece */
    } else if (is_castle(move)) {                 /* make reverse rook move */
        square_t rookfrom, rookto;
        if (to > from) {
            rookfrom = sq_rel(F1, us);
            rookto = sq_rel(H1, us);
        } else {
            rookfrom = sq_rel(D1, us);
            rookto = sq_rel(A1, us);
        }
        pos_set_sq(pos, rookto, pos->board[rookfrom]);
        pos_clr_sq(pos, rookfrom);
    } else if (is_enpassant(move)) {              /* restore grabbed pawn */
        square_t grabbed = to + up;
        pos_set_sq(pos, grabbed, MAKE_PIECE(PAWN, them));
    }

    pos->state = *state;                          /* restore irreversible changes */
    pos->turn = us;
    return pos;
}
