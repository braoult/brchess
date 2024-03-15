/* piece.h - piece definitions.
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

#ifndef PIECE_H
#define PIECE_H

#include <string.h>

#include "chessdefs.h"

/* piece_t bits structure
 * piece is on bits 1-3, color on bit 4:
 * .... CPPP
 * C: 0 for white, 1: black
 * PPP: pawn (1), knight, bishop, rook, queen, king (6)
 */
typedef enum {
    WHITE, BLACK,
    COLOR_MAX
} color_t;

typedef enum {
    ALL_PIECES = 0,                               /* 'all pieces' bitboard */
    NO_PIECE_TYPE = 0,
    PAWN = 1, KNIGHT, BISHOP, ROOK, QUEEN, KING,
    PIECE_TYPE_MAX = 7                            /* bit 4 */
} piece_type_t;

typedef enum __piece_e {
    EMPTY = 0,
    NO_PIECE = 0,
    W_PAWN = PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
    B_PAWN = PAWN | 8, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,
    PIECE_MAX
} piece_t;

/* default values for opening, midgame, endgame
 */
#define E_VAL_OPN    0                            /* empty */
#define P_VAL_OPN    100
#define N_VAL_OPN    300
#define B_VAL_OPN    300
#define R_VAL_OPN    500
#define Q_VAL_OPN    900
#define K_VAL_OPN    20000

#define E_VAL_MID    0
#define P_VAL_MID    100
#define N_VAL_MID    300
#define B_VAL_MID    300
#define R_VAL_MID    500
#define Q_VAL_MID    900
#define K_VAL_MID    20000

#define E_VAL_END    0
#define P_VAL_END    100
#define N_VAL_END    300
#define B_VAL_END    300
#define R_VAL_END    500
#define Q_VAL_END    900
#define K_VAL_END    20000

/* some default values for pieces
 * @abbr:   char, piece capital letter (used for game notation)
 * @abbr_c: char, capital for white, lowercase for black
 * char *sym;
 *  char *sym_c;
 *  char *name;
 *  s64  opn_value;
 *  s64  mid_value;
 *  s64  end_value;
 */
extern const struct piece_details {
    char *cap;                                    /* used for game notation */
    char *low;                                    /* used also for UCI promotion */
    char *fen;                                    /* cap=white, low=black */
    char *sym;                                    /* UTF-8 symbol */
    char *name;                                   /* piece name */
    s64  opn_value;                               /* value opening */
    s64  mid_value;                               /* value midgame */
    s64  end_value;                               /* value endgame */
} piece_details[PIECE_MAX];

extern const char pieces_str[6+6+1];              /* to search from fen/user input */

#define OPPONENT(color)   !(color)

#define MASK_PIECE        0x07                      /* 00000111 */
#define MASK_COLOR        0x08                      /* 00001000 */

#define COLOR(p)          ((p) >> 3)              /* bitmask */
#define PIECE(p)          ((p) & MASK_PIECE)
#define MAKE_PIECE(p, c)  ((p) | (c) << 3)

#define IS_WHITE(p)       (!COLOR(p))
#define IS_BLACK(p)       (COLOR(p))

#define SET_WHITE(p)      (piece_t)((p) &= ~MASK_COLOR)
#define SET_BLACK(p)      (piece_t)((p) |= MASK_COLOR)
#define SET_COLOR(p, c)   (piece_t)(!(c)? SET_WHITE(p): SET_BLACK(p))

bool piece_ok(piece_t p);

char *piece_to_cap(piece_t p);
char *piece_to_low(piece_t p);
char *piece_to_fen(piece_t p);
char *piece_to_sym(piece_t p);
char *piece_to_name(piece_t p);

#define piece_to_char(c) piece_to_fen(c)
//#define piece_to_char_t(p) piece_to_uci(p)

//piece_type_t char_to_piece(char c);
piece_type_t piece_t_from_char(char c);
piece_t piece_from_fen(char c);

#define piece_from_char(c) piece_from_fen(c)

/* use short name or symbol - no effect
 */
#define P_USE_UTF      1

//void piece_list_print(struct list_head *list);
//pool_t *piece_pool_init();
//void piece_pool_stats();
//piece_list_t *piece_add(pos_t *pos, piece_t piece, square_t square);
//void piece_del(struct list_head *ptr);
//int pieces_del(pos_t *pos, short color);

#endif
