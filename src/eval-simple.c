/* eval-simple.c - simple position evaluation.
 *
 * Copyright (C) 2023 Bruno Raoult ("br")
 * Licensed under the GNU General Public License v3.0 or later.
 * Some rights reserved. See COPYING.
 *
 * You should have received a copy of the GNU General Public License along with this
 * program. If not, see <https://www.gnu.org/licenses/gpl-3.0-standalone.html>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later <https://spdx.org/licenses/GPL-3.0-or-later.html>
 *
 */

#include <bits.h>
#include <debug.h>

#include "piece.h"
#include "eval-simple.h"
#include "position.h"

/*
 * Tables are from https://www.chessprogramming.org/Simplified_Evaluation_Function
 *
 * Attention! Tables are black point of view (to be visually easier to read).
 */

static int mg_pawn[] = {
    0,   0,   0,   0,   0,   0,  0,  0,
    50, 50,  50,  50,  50,  50, 50, 50,
    10, 10,  20,  30,  30,  20, 10, 10,
    5,   5,  10,  25,  25,  10,  5,  5,
    0,   0,   0,  20,  20,   0,  0,  0,
    5,  -5, -10,   0,   0, -10, -5,  5,
    5,  10,  10, -20, -20,  10, 10,  5,
    0,   0,   0,   0,   0,   0,  0,  0
};

static int mg_knight[] = {
    -50, -40, -30, -30, -30, -30, -40, -50,
    -40, -20,   0,   0,   0,   0, -20, -40,
    -30,   0,  10,  15,  15,  10,   0, -30,
    -30,   5,  15,  20,  20,  15,   5, -30,
    -30,   0,  15,  20,  20,  15,   0, -30,
    -30,   5,  10,  15,  15,  10,   5, -30,
    -40, -20,   0,   5,   5,   0, -20, -40,
    -50, -40, -30, -30, -30, -30, -40, -50
};

static int mg_bishop[] = {
    -20, -10, -10, -10, -10, -10, -10, -20,
    -10,   0,   0,   0,   0,   0,   0, -10,
    -10,   0,   5,  10,  10,   5,   0, -10,
    -10,   5,   5,  10,  10,   5,   5, -10,
    -10,   0,  10,  10,  10,  10,   0, -10,
    -10,  10,  10,  10,  10,  10,  10, -10,
    -10,   5,   0,   0,   0,   0,   5, -10,
    -20, -10, -10, -10, -10, -10, -10, -20
};

static int mg_rook[] = {
    0,   0,  0,  0,  0,  0,  0,  0,
    5,  10, 10, 10, 10, 10, 10,  5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    0,   0,  0,  5,  5,  0,  0,  0
};

static int mg_queen[] = {
    -20, -10, -10, -5, -5, -10, -10, -20,
    -10,   0,   0,  0,  0,   0,   0, -10,
    -10,   0,   5,  5,  5,   5,   0, -10,
    -5,    0,   5,  5,  5,   5,   0,  -5,
    0,     0,   5,  5,  5,   5,   0,  -5,
    -10,   5,   5,  5,  5,   5,   0, -10,
    -10,   0,   5,  0,  0,   0,   0, -10,
    -20, -10, -10, -5, -5, -10, -10, -20
};

static int mg_king[] = {
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -20, -30, -30, -40, -40, -30, -30, -20,
    -10, -20, -20, -20, -20, -20, -20, -10,
    20,  20,   0,   0,   0,   0,  20,  20,
    20,  30,  10,   0,   0,  10,  30,  20
};

static int eg_king[] = {
    -50, -40, -30, -20, -20, -30, -40, -50,
    -30, -20, -10,   0,   0, -10, -20, -30,
    -30, -10,  20,  30,  30,  20, -10, -30,
    -30, -10,  30,  40,  40,  30, -10, -30,
    -30, -10,  30,  40,  40,  30, -10, -30,
    -30, -10,  20,  30,  30,  20, -10, -30,
    -30, -30,   0,   0,   0,   0, -30, -30,
    -50, -30, -30, -30, -30, -30, -30, -50
};

/* as pieces bitboard tables start at position 2; we make these tables
 * bigger.
 */
static int *mg_tables[] = {
    NULL,
    NULL,
    mg_pawn,
    mg_knight,
    mg_bishop,
    mg_rook,
    mg_queen,
    mg_king
};

static int *eg_tables[] = {
    NULL,
    NULL,
    mg_pawn,
    mg_knight,
    mg_bishop,
    mg_rook,
    mg_queen,
    eg_king
};

/* to flip vertically a square, we need to XOR it with 56
 */
static int mg_table[2][6 + 2][64];
static int eg_table[2][6 + 2][64];

void eval_simple_init(void)
{
#   ifdef DEBUG_EVAL
    log_f(1, "initializing piece tables\n");
#   endif
    for (int piece = BB_PAWN; piece <= BB_KING; ++piece) {
        for (int square = 0; square < 64; ++square) {
            mg_table[WHITE][piece][square] = mg_tables[piece][FLIP_V(square)];
            eg_table[WHITE][piece][square] = eg_tables[piece][FLIP_V(square)];
            mg_table[BLACK][piece][square] = mg_tables[piece][square];
            eg_table[BLACK][piece][square] = eg_tables[piece][square];
        }
    }
}

/**
 * eval_simple() - simple and fast position evaluation
 * @pos: &position to evaluate
 *
 * This function is normally used only during initialization,
 * or when changing phase (middlegame <--> endgame), as the eval
 * will be done increntally when doing moves.
 *
 * @return: the @pos evaluation in centipawns
 */
eval_t eval_simple(pos_t *pos)
{
    eval_t eval[2] = { 0, 0 };
    int eg = simple_is_endgame(pos);
    int (*gg)[6 + 2][64]= eg? eg_table: mg_table;

    pos->eval_simple_phase = ENDGAME;
#   ifdef DEBUG_EVAL
    log_f(1, "phase = %s.\n", eg? "endgame": "midgame");
#   endif

    for (int color = WHITE; color <= BLACK; ++color) {
        for (uint piece = PAWN; piece <= KING; piece <<= 1) {
            int bb = PIECETOBB(piece), cur;
            u64 _t;

#           ifdef DEBUG_EVAL
            log_f(1, "p=%u bb=%d %s %s: count=%d val=%ld ", piece, bb, color? "black": "white",
                  P_SYM(piece), popcount64(pos->bb[color][bb]),
                  popcount64(pos->bb[color][bb]) * P_VALUE(piece));
#           endif

            eval[color] += popcount64(pos->bb[color][bb]) * P_LETTER(piece);
            bit_for_each64_2(cur, _t, pos->bb[color][piece]) {
#               ifdef DEBUG_EVAL
                log(1, "sq=%d:%d ", cur, gg[color][bb][cur]);
#               endif
                eval[color] += gg[color][bb][cur];
            }
#           ifdef DEBUG_EVAL
            log(1, "\n");
#           endif
        }
    }
#   ifdef DEBUG_EVAL
    log_f(1, "white: %d black:%d\n", eval[WHITE], eval[BLACK]);
#   endif

    return eval[WHITE] - eval[BLACK];
}
