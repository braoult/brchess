/* position.h - position management definitions.
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

#ifndef POSITION_H
#define POSITION_H

#include <stdint.h>

#include <brlib.h>
#include <bitops.h>
#include <struct-group.h>
#include <bug.h>

#include "chessdefs.h"
#include "hash.h"
#include "bitboard.h"
#include "piece.h"
#include "move.h"
#include "board.h"

typedef struct __pos_s {
    u64 node_count;                               /* evaluated nodes */
    int turn;                                     /* WHITE or BLACK */

    /* data which cannot be recovered by move_undo (like castle_rights, ...).
     *
     * Attention: checkers/pinners/blockers are not included here, and
     * are not available in move_undo or any following legality check.
     *
     * Following data can be accessed either directly, either via "state"
     * structure name.
     * For example, pos->en_passant and pos->state.en_passant are the same.
     * This allows a memcpy on this data (to save/restore position state).
     */
    struct_group_tagged(state_s, state,
                        key_t key;
                        square_t en_passant;
                        castle_rights_t castle;
                        int clock_50;
                        int plycount;             /* plies so far, start from 1 */
                        piece_t captured;         /* only used in move_undo */
        );
    bitboard_t checkers;                          /* opponent checkers */
    bitboard_t pinners;                           /* opponent pinners */
    bitboard_t blockers;                          /* pieces blocking pin */

    piece_t board[BOARDSIZE];
    bitboard_t bb[2][PIECE_TYPE_MAX];             /* bb[0][PAWN], bb[1][ALL_PIECES] */
    square_t king[2];                             /* dup with bb, faster retrieval */
} pos_t;

typedef struct state_s state_t;

#define pos_pinned(p)                  (p->blockers & p->bb[p->turn][ALL_PIECES])

/**
 * pos_set_sq - unconditionally set a piece on a square
 * @pos: position
 * @square: square_t to set
 * @piece: piece_t to add
 *
 * Both position board and bitboards are modified.
 */
static __always_inline void pos_set_sq(pos_t *pos, square_t square, piece_t piece)
{
    color_t color = COLOR(piece);
    piece_type_t type = PIECE(piece);

    bug_on(pos->board[square] != EMPTY);

    pos->board[square] = piece;
    pos->bb[color][type] |= BIT(square);
    pos->bb[color][ALL_PIECES] |= BIT(square);
    //if (type == KING)
    //    pos->king[color] = square;

    //pos->key ^= zobrist_pieces[piece][square];
}

/**
 * pos_clr_sq - unconditionally remove a piece from square
 * @pos: position
 * @square: square_t to clear
 *
 * Both position board and bitboards are modified.
 */
static __always_inline void pos_clr_sq(pos_t *pos, square_t square)
{
    piece_t piece = pos->board[square];
    piece_type_t type = PIECE(piece);
    color_t color = COLOR(piece);

    bug_on(pos->board[square] == EMPTY);

    //pos->key ^= zobrist_pieces[piece][square];

    pos->board[square] = EMPTY;
    pos->bb[color][type] &= ~BIT(square);
    pos->bb[color][ALL_PIECES] &= ~BIT(square);
    //if (type == KING)
    //    pos->king[color] = SQUARE_NONE;
}

/**
 * pos_occ() - get position occupation (all pieces)
 * @pos: position
 *
 * @return: occupation bitboard.
 */
static __always_inline bitboard_t pos_occ(const pos_t *pos)
{
    return pos->bb[WHITE][ALL_PIECES] | pos->bb[BLACK][ALL_PIECES];
}

/**
 * pos_between_occ() - find occupation between two squares.
 * @pos:   position
 * @sq1:   square 1
 * @sq2:   square 2
 *
 * @return: bitboard of @betw if between @sq1 and @sq2.
 */
static __always_inline bitboard_t pos_between_occ(const pos_t *pos,
                                                  const square_t sq1, const square_t sq2)
{
    return bb_between_excl[sq1][sq2] & pos_occ(pos);
}

/**
 * pos_between_count() - count occupied squares between two squares.
 * @pos:   position
 * @sq1:   square 1
 * @sq2:   square 2
 *
 * @return: bitboard of @betw if between @sq1 and @sq2.
 */
static __always_inline int pos_between_count(const pos_t *pos,
                                             const square_t sq1, const square_t sq2)
{
    return popcount64(pos_between_occ(pos, sq1, sq2));
}

/**
 * pos_checkers2str() - get of string of checkers.
 * @p:   position
 * @str:   destination string
 * @len:   max @str len.
 *
 * A wrapper over @bb_sq2str() for checkers bitmap.
 *
 * @return: @str.
 */
#define pos_checkers2str(p, str, len)  bb_sq2str(p->checkers, str, len)
#define pos_pinners2str(p, str, len)   bb_sq2str(p->pinners, str, len)
#define pos_blockers2str(p, str, len)  bb_sq2str(p->blockers, str, len)

//void bitboard_print(bitboard_t bb, char *title);
//void bitboard_print2(bitboard_t bb1, bitboard_t bb2, char *title);

pos_t *pos_new();
pos_t *pos_dup(const pos_t *pos);
pos_t *pos_copy(const pos_t *from, pos_t *to);
void pos_del(pos_t *pos);
pos_t *pos_clear(pos_t *pos);
bool pos_cmp(const pos_t *pos1, const pos_t *pos2);

void pos_set_checkers_pinners_blockers(pos_t *pos);
void pos_set_pinners_blockers(pos_t *pos);
bitboard_t pos_checkers(const pos_t *pos, const color_t color);
bitboard_t pos_king_pinners(const pos_t *pos, const color_t color);
bitboard_t pos_king_blockers(const pos_t *pos, const color_t color, const bitboard_t );

bool pos_ok(pos_t *pos, const bool strict);

void pos_print(const pos_t *pos);
void pos_print_mask(const pos_t *pos, const bitboard_t mask);
void pos_print_raw(const pos_t *pos, const int type);

void pos_print_pieces(const pos_t *pos);


#endif  /* POSITION_H */
