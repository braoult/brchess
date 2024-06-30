/* piece.c - piece list management.
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
#include <string.h>
#include <assert.h>

#include "bug.h"

#include "chessdefs.h"
#include "piece.h"

/**
 * piece_details
 */
const struct piece_details piece_details[PIECE_MAX] = {
    /*             cap  low  fen  sym  name      midgame val endgame val */
    [EMPTY]    = { "",  "",  "",  "",  "",       0,          0 },
    [W_PAWN]   = { "",  "",  "P", "♙", "Pawn",   P_VAL_MID,  P_VAL_END },
    [W_KNIGHT] = { "N", "n", "N", "♘", "Knight", N_VAL_MID,  N_VAL_END },
    [W_BISHOP] = { "B", "b", "B", "♗", "Bishop", B_VAL_MID,  B_VAL_END },
    [W_ROOK]   = { "R", "r", "R", "♖", "Rook",   R_VAL_MID,  R_VAL_END },
    [W_QUEEN]  = { "Q", "q", "Q", "♕", "Queen",  Q_VAL_MID,  Q_VAL_END },
    [W_KING]   = { "K", "k", "K", "♔", "King",   K_VAL_MID,  K_VAL_END },
    [7]        = { "",  "",  "",  "",  "",       0,          0         },
    [8]        = { "",  "",  "",  "",  "",       0,          0         },
    [B_PAWN]   = { "",  "",  "p", "♟", "Pawn",   P_VAL_MID,  P_VAL_END },
    [B_KNIGHT] = { "N", "n", "n", "♞", "Knight", N_VAL_MID,  N_VAL_END },
    [B_BISHOP] = { "B", "b", "b", "♝", "Bishop", B_VAL_MID,  B_VAL_END },
    [B_ROOK]   = { "R", "r", "r", "♜", "Rook",   R_VAL_MID,  R_VAL_END },
    [B_QUEEN]  = { "Q", "q", "q", "♛", "Queen",  Q_VAL_MID,  Q_VAL_END },
    [B_KING]   = { "K", "k", "k", "♚", "King",   K_VAL_MID,  K_VAL_END },
};

const char pieces_str[6+6+1] = "PNBRQKpnbrqk";

bool piece_ok(piece_t p)
{
    piece_type_t pt = PIECE(p);
    return !(p & ~(MASK_COLOR | MASK_PIECE)) && pt && (pt <= KING);
}

char *piece_to_cap(piece_t p)
{
    return piece_details[p].cap;
}

char *piece_to_char(piece_t p)
{
    return piece_details[p].fen;
}

char *piece_to_low(piece_t p)
{
    return piece_details[p].low;
}

char *piece_to_sym(piece_t p)
{
    return piece_details[p].sym;
}

char *piece_to_name(piece_t p)
{
    return piece_details[p].name;
}

piece_type_t piece_t_from_char(char c)
{
    char *p;
    piece_type_t pt = NO_PIECE_TYPE;
    if (c && (p = strchr(pieces_str, c))) {
        pt = (p - pieces_str) % 6 + 1;
    }
    return pt;
}

//piece_type_t piece_from_promotion(char c, color_t color)
//{
//    piece_type_t piece = piece_t_from_char(c);
//    return piece? SET_COLOR()p? (p - pieces_str) % 6 + 1: NO_PIECE_TYPE;
//}

piece_t piece_from_char(char c)
{
    piece_type_t piece = piece_t_from_char(c);
    return isupper(c)? SET_WHITE(piece): SET_BLACK(piece);
}
