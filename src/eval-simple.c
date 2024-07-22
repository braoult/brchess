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

#include <brlib.h>
#include <bitops.h>

#include "chessdefs.h"
#include "piece.h"
#include "position.h"
#include "eval-simple.h"
#include "eval.h"

/*
 * Piece-square tables. For easier reading, they are defined for black side:
 *
 *      {
 *        A8 .... H8
 *        ..........
 *        ..........
 *        A1 .... H1
 *      }
 */
const struct pc_sq pc_sq_def[] = {
    {
        /*
         * rofchade:
         * https://www.talkchess.com/forum3/viewtopic.php?f=2&t=68311&start=19
         */
        "rofchade",
        {
            [PAWN] = {
                {                                 /* midgame */
                    0,   0,   0,   0,   0,   0,  0,   0,
                    98, 134,  61,  95,  68, 126, 34, -11,
                    -6,   7,  26,  31,  65,  56, 25, -20,
                    -14,  13,   6,  21,  23,  12, 17, -23,
                    -27,  -2,  -5,  12,  17,   6, 10, -25,
                    -26,  -4,  -4, -10,   3,   3, 33, -12,
                    -35,  -1, -20, -23, -15,  24, 38, -22,
                    0,   0,   0,   0,   0,   0,  0,   0,
                },
                {                                 /* endgame */
                    0,   0,   0,   0,   0,   0,   0,   0,
                    178, 173, 158, 134, 147, 132, 165, 187,
                    94, 100,  85,  67,  56,  53,  82,  84,
                    32,  24,  13,   5,  -2,   4,  17,  17,
                    13,   9,  -3,  -7,  -7,  -8,   3,  -1,
                    4,   7,  -6,   1,   0,  -5,  -1,  -8,
                    13,   8,   8,  10,  13,   0,   2,  -7,
                    0,   0,   0,   0,   0,   0,   0,   0,
                }
            },
            [KNIGHT] = {
                {                                 /* midgame */
                    -167, -89, -34, -49,  61, -97, -15, -107,
                    -73, -41,  72,  36,  23,  62,   7,  -17,
                    -47,  60,  37,  65,  84, 129,  73,   44,
                    -9,  17,  19,  53,  37,  69,  18,   22,
                    -13,   4,  16,  13,  28,  19,  21,   -8,
                    -23,  -9,  12,  10,  19,  17,  25,  -16,
                    -29, -53, -12,  -3,  -1,  18, -14,  -19,
                    -105, -21, -58, -33, -17, -28, -19,  -23,
                },
                {                                 /* endgame */
                    -58, -38, -13, -28, -31, -27, -63, -99,
                    -25,  -8, -25,  -2,  -9, -25, -24, -52,
                    -24, -20,  10,   9,  -1,  -9, -19, -41,
                    -17,   3,  22,  22,  22,  11,   8, -18,
                    -18,  -6,  16,  25,  16,  17,   4, -18,
                    -23,  -3,  -1,  15,  10,  -3, -20, -22,
                    -42, -20, -10,  -5,  -2, -20, -23, -44,
                    -29, -51, -23, -15, -22, -18, -50, -64,
                }
            },
            [BISHOP] = {
                {                                 /* midgame */
                    -29,  4, -82, -37, -25, -42,   7,  -8,
                    -26, 16, -18, -13,  30,  59,  18, -47,
                    -16, 37,  43,  40,  35,  50,  37,  -2,
                     -4,  5,  19,  50,  37,  37,   7,  -2,
                     -6, 13,  13,  26,  34,  12,  10,   4,
                      0, 15,  15,  15,  14,  27,  18,  10,
                      4, 15,  16,   0,   7,  21,  33,   1,
                    -33, -3, -14, -21, -13, -12, -39, -21,
                },
                {                                 /* endgame */
                    -14, -21, -11,  -8, -7,  -9, -17, -24,
                     -8,  -4,   7, -12, -3, -13,  -4, -14,
                      2,  -8,   0,  -1, -2,   6,   0,   4,
                     -3,   9,  12,   9, 14,  10,   3,   2,
                     -6,   3,  13,  19,  7,  10,  -3,  -9,
                    -12,  -3,   8,  10, 13,   3,  -7, -15,
                    -14, -18,  -7,  -1,  4,  -9, -15, -27,
                    -23,  -9, -23,  -5, -9, -16,  -5, -17,
                },
            },
            [ROOK] = {
                {                                 /* midgame */
                    32,  42,  32,  51, 63,  9,  31,  43,
                    27,  32,  58,  62, 80, 67,  26,  44,
                    -5,  19,  26,  36, 17, 45,  61,  16,
                    -24, -11,   7,  26, 24, 35,  -8, -20,
                    -36, -26, -12,  -1,  9, -7,   6, -23,
                    -45, -25, -16, -17,  3,  0,  -5, -33,
                    -44, -16, -20,  -9, -1, 11,  -6, -71,
                    -19, -13,   1,  17, 16,  7, -37, -26,
                },
                {                                 /* endgame */
                    13, 10, 18, 15, 12,  12,   8,   5,
                    11, 13, 13, 11, -3,   3,   8,   3,
                    7,  7,  7,  5,  4,  -3,  -5,  -3,
                    4,  3, 13,  1,  2,   1,  -1,   2,
                    3,  5,  8,  4, -5,  -6,  -8, -11,
                    -4,  0, -5, -1, -7, -12,  -8, -16,
                    -6, -6,  0,  2, -9,  -9, -11,  -3,
                    -9,  2,  3, -1, -5, -13,   4, -20,
                },
            },
            [QUEEN] = {
                {                                 /* midgame */
                    -28,   0,  29,  12,  59,  44,  43,  45,
                    -24, -39,  -5,   1, -16,  57,  28,  54,
                    -13, -17,   7,   8,  29,  56,  47,  57,
                    -27, -27, -16, -16,  -1,  17,  -2,   1,
                    -9, -26,  -9, -10,  -2,  -4,   3,  -3,
                    -14,   2, -11,  -2,  -5,   2,  14,   5,
                    -35,  -8,  11,   2,   8,  15,  -3,   1,
                    -1, -18,  -9,  10, -15, -25, -31, -50,
                },
                {                                 /* endgame */
                    -9,  22,  22,  27,  27,  19,  10,  20,
                    -17,  20,  32,  41,  58,  25,  30,   0,
                    -20,   6,   9,  49,  47,  35,  19,   9,
                    3,  22,  24,  45,  57,  40,  57,  36,
                    -18,  28,  19,  47,  31,  34,  39,  23,
                    -16, -27,  15,   6,   9,  17,  10,   5,
                    -22, -23, -30, -16, -16, -23, -36, -32,
                    -33, -28, -22, -43,  -5, -32, -20, -41,
                },
            },
            [KING] = {                            /* midgame */
                {
                    -65,  23,  16, -15, -56, -34,   2,  13,
                    29,  -1, -20,  -7,  -8,  -4, -38, -29,
                    -9,  24,   2, -16, -20,   6,  22, -22,
                    -17, -20, -12, -27, -30, -25, -14, -36,
                    -49,  -1, -27, -39, -46, -44, -33, -51,
                    -14, -14, -22, -46, -44, -30, -15, -27,
                    1,   7,  -8, -64, -43, -16,   9,   8,
                    -15,  36,  12, -54,   8, -28,  24,  14,
                },
                {                                 /* endgame */
                    -74, -35, -18, -18, -11,  15,   4, -17,
                    -12,  17,  14,  17,  17,  38,  23,  11,
                    10,  17,  23,  15,  20,  45,  44,  13,
                    -8,  22,  24,  27,  26,  33,  26,   3,
                    -18,  -4,  21,  24,  27,  23,   9, -11,
                    -19,  -3,  11,  21,  23,  16,   7,  -9,
                    -27, -11,   4,  13,  14,   4,  -5, -17,
                    -53, -34, -21, -11, -28, -14, -24, -43
                }
            }
        }
    },                                            /* CPW */
    {
        /*
         * CPW:
         * https://www.chessprogramming.org/Simplified_Evaluation_Function
         * Note: ≠ https://github.com/nescitus/cpw-engine
         */
        "cpw",
        {
            [PAWN] = {
                {                                 /* midgame */
                    0,  0,   0,   0,   0,   0,  0,  0,
                    50, 50,  50,  50,  50,  50, 50, 50,
                    10, 10,  20,  30,  30,  20, 10, 10,
                    5,  5,  10,  25,  25,  10,  5,  5,
                    0,  0,   0,  20,  20,   0,  0,  0,
                    5, -5, -10,   0,   0, -10, -5,  5,
                    5, 10,  10, -20, -20,  10, 10,  5,
                    0,  0,   0,   0,   0,   0,  0,  0,
                },
                {                                 /* endgame */
                    0,  0,   0,   0,   0,   0,  0,  0,
                    50, 50,  50,  50,  50,  50, 50, 50,
                    10, 10,  20,  30,  30,  20, 10, 10,
                    5,  5,  10,  25,  25,  10,  5,  5,
                    0,  0,   0,  20,  20,   0,  0,  0,
                    5, -5, -10,   0,   0, -10, -5,  5,
                    5, 10,  10, -20, -20,  10, 10,  5,
                    0,  0,   0,   0,   0,   0,  0,  0,
                },
            },
            [KNIGHT] = {
                {                                 /* midgame */
                    -50, -40, -30, -30, -30, -30, -40, -50,
                    -40, -20,   0,   0,   0,   0, -20, -40,
                    -30,   0,  10,  15,  15,  10,   0, -30,
                    -30,   5,  15,  20,  20,  15,   5, -30,
                    -30,   0,  15,  20,  20,  15,   0, -30,
                    -30,   5,  10,  15,  15,  10,   5, -30,
                    -40, -20,   0,   5,   5,   0, -20, -40,
                    -50, -40, -30, -30, -30, -30, -40, -50
                },
                {                                 /* endgame */
                    -50, -40, -30, -30, -30, -30, -40, -50,
                    -40, -20,   0,   0,   0,   0, -20, -40,
                    -30,   0,  10,  15,  15,  10,   0, -30,
                    -30,   5,  15,  20,  20,  15,   5, -30,
                    -30,   0,  15,  20,  20,  15,   0, -30,
                    -30,   5,  10,  15,  15,  10,   5, -30,
                    -40, -20,   0,   5,   5,   0, -20, -40,
                    -50, -40, -30, -30, -30, -30, -40, -50
                },
            },
            [BISHOP] = {
                {                                 /* midgame */
                    -20, -10, -10, -10, -10, -10, -10, -20,
                    -10,   0,   0,   0,   0,   0,   0, -10,
                    -10,   0,   5,  10,  10,   5,   0, -10,
                    -10,   5,   5,  10,  10,   5,   5, -10,
                    -10,   0,  10,  10,  10,  10,   0, -10,
                    -10,  10,  10,  10,  10,  10,  10, -10,
                    -10,   5,   0,   0,   0,   0,   5, -10,
                    -20, -10, -10, -10, -10, -10, -10, -20,
                },
                {                                 /* endgame */
                    -20, -10, -10, -10, -10, -10, -10, -20,
                    -10,   0,   0,   0,   0,   0,   0, -10,
                    -10,   0,   5,  10,  10,   5,   0, -10,
                    -10,   5,   5,  10,  10,   5,   5, -10,
                    -10,   0,  10,  10,  10,  10,   0, -10,
                    -10,  10,  10,  10,  10,  10,  10, -10,
                    -10,   5,   0,   0,   0,   0,   5, -10,
                    -20, -10, -10, -10, -10, -10, -10, -20,
                },
            },
            [ROOK] = {
                {                                 /* midgame */
                    0,  0,  0,  0,  0,  0,  0,  0,
                    5, 10, 10, 10, 10, 10, 10,  5,
                    -5,  0,  0,  0,  0,  0,  0, -5,
                    -5,  0,  0,  0,  0,  0,  0, -5,
                    -5,  0,  0,  0,  0,  0,  0, -5,
                    -5,  0,  0,  0,  0,  0,  0, -5,
                    -5,  0,  0,  0,  0,  0,  0, -5,
                    0,  0,  0,  5,  5,  0,  0,  0,
                },
                {                                 /* endgame */
                    0,  0,  0,  0,  0,  0,  0,  0,
                    5, 10, 10, 10, 10, 10, 10,  5,
                    -5,  0,  0,  0,  0,  0,  0, -5,
                    -5,  0,  0,  0,  0,  0,  0, -5,
                    -5,  0,  0,  0,  0,  0,  0, -5,
                    -5,  0,  0,  0,  0,  0,  0, -5,
                    -5,  0,  0,  0,  0,  0,  0, -5,
                    0,  0,  0,  5,  5,  0,  0,  0,
                },
            },
            [QUEEN] = {
                {                                 /* midgame */
                    -20, -10, -10, -5, -5, -10, -10, -20,
                    -10,   0,   0,  0,  0,   0,   0, -10,
                    -10,   0,   5,  5,  5,   5,   0, -10,
                    -5,   0,   5,  5,  5,   5,   0,  -5,
                    0,   0,   5,  5,  5,   5,   0,  -5,
                    -10,   5,   5,  5,  5,   5,   0, -10,
                    -10,   0,   5,  0,  0,   0,   0, -10,
                    -20, -10, -10, -5, -5, -10, -10, -20,
                },
                {                                 /* endgame */
                    -20, -10, -10, -5, -5, -10, -10, -20,
                    -10,   0,   0,  0,  0,   0,   0, -10,
                    -10,   0,   5,  5,  5,   5,   0, -10,
                    -5,   0,   5,  5,  5,   5,   0,  -5,
                    0,   0,   5,  5,  5,   5,   0,  -5,
                    -10,   5,   5,  5,  5,   5,   0, -10,
                    -10,   0,   5,  0,  0,   0,   0, -10,
                    -20, -10, -10, -5, -5, -10, -10, -20,
                },
            },
            [KING] = {                            /* midgame */
                {
                    -30, -40, -40, -50, -50, -40, -40, -30,
                    -30, -40, -40, -50, -50, -40, -40, -30,
                    -30, -40, -40, -50, -50, -40, -40, -30,
                    -30, -40, -40, -50, -50, -40, -40, -30,
                    -20, -30, -30, -40, -40, -30, -30, -20,
                    -10, -20, -20, -20, -20, -20, -20, -10,
                    20,  20,   0,   0,   0,   0,  20,  20,
                    20,  30,  10,   0,   0,  10,  30,  20,
                },
                {                                 /* endgame */
                    -50, -40, -30, -20, -20, -30, -40, -50,
                    -30, -20, -10,   0,   0, -10, -20, -30,
                    -30, -10,  20,  30,  30,  20, -10, -30,
                    -30, -10,  30,  40,  40,  30, -10, -30,
                    -30, -10,  30,  40,  40,  30, -10, -30,
                    -30, -10,  20,  30,  30,  20, -10, -30,
                    -30, -30,   0,   0,   0,   0, -30, -30,
                    -50, -30, -30, -30, -30, -30, -30, -50,
                }
            }
        }
    },                                            /* CPW */
    {
        /*
         * sjeng: https://github.com/gcp/sjeng
         * Rook and Queen from CPW.
         */
        "sjeng",
        {
            [PAWN] = {
                {                                 /* midgame */
                    0,  0,  0,  0,  0,  0,  0, 0,
                    5, 10, 15, 20, 20, 15, 10, 5,
                    4,  8, 12, 16, 16, 12,  8, 4,
                    3,  6,  9, 14, 14,  9,  6, 3,
                    2,  4,  6, 12, 12,  6,  4, 2,
                    1,  2,  3, 10, 10,  3,  2, 1,
                    0,  0,  0,  0,  0,  0,  0, 0,
                    0,  0,  0,  0,  0,  0,  0, 0,
                },
                {                                 /* endgame */
                    0,  0,  0,  0,  0,  0,  0, 0,
                    5, 10, 15, 20, 20, 15, 10, 5,
                    4,  8, 12, 16, 16, 12,  8, 4,
                    3,  6,  9, 14, 14,  9,  6, 3,
                    2,  4,  6, 12, 12,  6,  4, 2,
                    1,  2,  3, 10, 10,  3,  2, 1,
                    0,  0,  0,  0,  0,  0,  0, 0,
                    0,  0,  0,  0,  0,  0,  0, 0,
                },
            },
            [KNIGHT] = {
                {                                 /* midgame */
                    -20, -10, -10, -10, -10, -10, -10, -20,
                    -10,   0,   0,   3,   3,   0,   0, -10,
                    -10,   0,   5,   5,   5,   5,   0, -10,
                    -10,   0,   5,  10,  10,   5,   0, -10,
                    -10,   0,   5,  10,  10,   5,   0, -10,
                    -10,   0,   5,   5,   5,   5,   0, -10,
                    -10,   0,   0,   3,   3,   0,   0, -10,
                    -20, -10, -10, -10, -10, -10, -10, -20,
                },
                {                                 /* endgame */
                    -20, -10, -10, -10, -10, -10, -10, -20,
                    -10,   0,   0,   3,   3,   0,   0, -10,
                    -10,   0,   5,   5,   5,   5,   0, -10,
                    -10,   0,   5,  10,  10,   5,   0, -10,
                    -10,   0,   5,  10,  10,   5,   0, -10,
                    -10,   0,   5,   5,   5,   5,   0, -10,
                    -10,   0,   0,   3,   3,   0,   0, -10,
                    -20, -10, -10, -10, -10, -10, -10, -20,
                },
            },
            [BISHOP] = {
                {                                 /* midgame */
                    -2, -2, -2, -2, -2, -2, -2, -2,
                    -2,  8,  5,  5,  5,  5,  8, -2,
                    -2,  3,  3,  5,  5,  3,  3, -2,
                    -2,  2,  5,  4,  4,  5,  2, -2,
                    -2,  2,  5,  4,  4,  5,  2, -2,
                    -2,  3,  3,  5,  5,  3,  3, -2,
                    -2,  8,  5,  5,  5,  5,  8, -2,
                    -2, -2, -2, -2, -2, -2, -2, -2,
                },
                {                                 /* endgame */
                    -2, -2, -2, -2, -2, -2, -2, -2,
                    -2,  8,  5,  5,  5,  5,  8, -2,
                    -2,  3,  3,  5,  5,  3,  3, -2,
                    -2,  2,  5,  4,  4,  5,  2, -2,
                    -2,  2,  5,  4,  4,  5,  2, -2,
                    -2,  3,  3,  5,  5,  3,  3, -2,
                    -2,  8,  5,  5,  5,  5,  8, -2,
                    -2, -2, -2, -2, -2, -2, -2, -2,
                },
            },
            [ROOK] = {
                {                                 /* midgame */
                    0,  0,  0,  0,  0,  0,  0,  0,
                    5, 10, 10, 10, 10, 10, 10,  5,
                    -5,  0,  0,  0,  0,  0,  0, -5,
                    -5,  0,  0,  0,  0,  0,  0, -5,
                    -5,  0,  0,  0,  0,  0,  0, -5,
                    -5,  0,  0,  0,  0,  0,  0, -5,
                    -5,  0,  0,  0,  0,  0,  0, -5,
                    0,  0,  0,  5,  5,  0,  0,  0,
                },
                {                                 /* endgame */
                    0,  0,  0,  0,  0,  0,  0,  0,
                    5, 10, 10, 10, 10, 10, 10,  5,
                    -5,  0,  0,  0,  0,  0,  0, -5,
                    -5,  0,  0,  0,  0,  0,  0, -5,
                    -5,  0,  0,  0,  0,  0,  0, -5,
                    -5,  0,  0,  0,  0,  0,  0, -5,
                    -5,  0,  0,  0,  0,  0,  0, -5,
                    0,  0,  0,  5,  5,  0,  0,  0,
                },
            },
            [QUEEN] = {
                {                                 /* midgame */
                    -20, -10, -10, -5, -5, -10, -10, -20,
                    -10,   0,   5,  0,  0,   0,   0, -10,
                    -10,   5,   5,  5,  5,   5,   0, -10,
                    0,   0,   5,  5,  5,   5,   0,  -5,
                    -5,   0,   5,  5,  5,   5,   0,  -5,
                    -10,   0,   5,  5,  5,   5,   0, -10,
                    -10,   0,   0,  0,  0,   0,   0, -10,
                    -20, -10, -10, -5, -5, -10, -10, -20,
                },
                {                                 /* endgame */
                    -20, -10, -10, -5, -5, -10, -10, -20,
                    -10,   0,   5,  0,  0,   0,   0, -10,
                    -10,   5,   5,  5,  5,   5,   0, -10,
                    0,   0,   5,  5,  5,   5,   0,  -5,
                    -5,   0,   5,  5,  5,   5,   0,  -5,
                    -10,   0,   5,  5,  5,   5,   0, -10,
                    -10,   0,   0,  0,  0,   0,   0, -10,
                    -20, -10, -10, -5, -5, -10, -10, -20,
                },
            },
            [KING] = {                            /* midgame */
                {
                    -55, -55, -89, -89, -89, -89, -55, -55,
                    -34, -34, -55, -55, -55, -55, -34, -34,
                    -21, -21, -34, -34, -34, -34, -21, -21,
                    -13, -13, -21, -21, -21, -21, -13, -13,
                    -8,  -8, -13, -13, -13, -13,  -8,  -8,
                    -5,  -5,  -8,  -8,  -8,  -8,  -5,  -5,
                    -3,  -5,  -6,  -6,  -6,  -6,  -5,  -3,
                    2,  14,   0,   0,   0,   9,  14,   2,
                },
                {                                 /* endgame */
                    -5, -3, -1,  0,  0, -1, -3, -5,
                    -3, 10, 10, 10, 10, 10, 10, -3,
                    -1, 10, 25, 25, 25, 25, 10, -1,
                    0, 10, 25, 30, 30, 25, 10,  0,
                    0, 10, 25, 30, 30, 25, 10,  0,
                    -1, 10, 25, 25, 25, 25, 10, -1,
                    -3, 10, 10, 10, 10, 10, 10, -3,
                    -5, -3, -1,  0,  0, -1, -3, -5,
                }
            }
        },
    },                                            /* sjeng */
};

const int nb_pc_sq = ARRAY_SIZE(pc_sq_def);       /* # of predefined pc_sq tables */
int pc_sq_current = 0;
int pc_sq_mg[COLOR_NB][PT_NB][SQUARE_NB];
int pc_sq_eg[COLOR_NB][PT_NB][SQUARE_NB];

/* phase calculation from Fruit:
 * https://github.com/Warpten/Fruit-2.1
*/

/**
 * calc_phase - calculate position phase
 * @pos: &position
 *
 * This function should be calculated when a new position is setup, or as
 * a verification of an incremental one.
 * phase is clamped between 0 (opening) and 24 (ending).
 *
 * @return:
 */
s16 calc_phase(pos_t *pos)
{
    int phase = ALL_PHASE;
    phase -= P_PHASE * popcount64(pos->bb[WHITE][PAWN] | pos->bb[BLACK][PAWN]);
    phase -= N_PHASE * popcount64(pos->bb[WHITE][KNIGHT] | pos->bb[BLACK][KNIGHT]);
    phase -= B_PHASE * popcount64(pos->bb[WHITE][BISHOP] | pos->bb[BLACK][BISHOP]);
    phase -= R_PHASE * popcount64(pos->bb[WHITE][ROOK] | pos->bb[BLACK][ROOK]);
    phase -= Q_PHASE * popcount64(pos->bb[WHITE][QUEEN] | pos->bb[BLACK][QUEEN]);

    phase = clamp(phase, 0, ALL_PHASE);
#   ifdef DEBUG_EVAL
    printf("calc phase:%d\n", phase);
#   endif
    return phase;
}

int eval_simple_find(char *str)
{
    for (int i = 0; i < nb_pc_sq; ++i)
        if (!strcmp(pc_sq_def[i].name, str))
            return i;
    return -1;
}

void eval_simple_set(int set)
{
    const struct pc_sq *cur = pc_sq_def + set;
#   ifdef DEBUG_EVAL
    printf("initializing piece-square tables %d\n", set);
#   endif
    pc_sq_current = set;
    for (piece_type_t pt = PAWN; pt < PT_NB; ++pt) {
        for (square_t sq = 0; sq < SQUARE_NB; ++sq) {
            pc_sq_mg[BLACK][pt][sq] = cur->val[MIDGAME][pt][sq];
            pc_sq_mg[WHITE][pt][sq] = cur->val[MIDGAME][pt][FLIP_V(sq)];

            pc_sq_eg[BLACK][pt][sq] = cur->val[ENDGAME][pt][sq];
            pc_sq_eg[WHITE][pt][sq] = cur->val[ENDGAME][pt][FLIP_V(sq)];
        }
    }
}

void eval_simple_init(char *set)
{
    int n = eval_simple_find(set);
#   ifdef DEBUG_EVAL
    printf("initializing eval_simple with set=%s: found=%d\n", set, n);
#   endif

    if (n >= 0)
        pc_sq_current = n;
    eval_simple_set(pc_sq_current);
}

/**
 * eval_material() - eval position material
 * @pos: &position to evaluate
 *
 * Basic material evaluation. Only midgame value is used.
 *
 * @return: the @pos material evaluation in centipawns
 */
eval_t eval_material(pos_t *pos)
{
    eval_t val[COLOR_NB] = { 0 };

    for (piece_type_t pt = PAWN; pt < KING; ++pt) {
        eval_t pval = piece_midval(pt);
        val[WHITE] += popcount64(pos->bb[WHITE][pt]) * pval;
        val[BLACK] += popcount64(pos->bb[BLACK][pt]) * pval;
    }
#   ifdef DEBUG_EVAL
    printf("material: w:%d b:%d\n", val[WHITE], val[BLACK]);
#   endif

    return val[WHITE] - val[BLACK];
}

/**
 * eval_simple() - simple and fast position evaluation
 * @pos: &position to evaluate
 *
 * This function is normally used only during initialization,
 * or when changing phase (middlegame <--> endgame), as the eval
 * will be done incrementally when doing moves.
 *
 * @return: the @pos evaluation in centipawns
 */
eval_t eval_simple(pos_t *pos)
{
    eval_t eval[2] = { 0, 0 };
    eval_t mg_eval[2], eg_eval[2];
    //struct pc_sq = sq_ int (*gg)[6 + 2][64] = eg? pc_sq_eg: pc_sq_mg;

    //pos->eval_simple_phase = ENDGAME;

    for (color_t color = WHITE; color < COLOR_NB; ++color) {
        mg_eval[color] = 0;
        eg_eval[color] = 0;
        for (piece_type_t pt = PAWN; pt < KING; pt++) {
            bitboard_t bb = pos->bb[color][pt];
            while (bb) {
                square_t sq = bb_next(&bb);
                mg_eval[color] += pc_sq_mg[color][pt][sq];
                eg_eval[color] += pc_sq_eg[color][pt][sq];
            }

#           ifdef DEBUG_EVAL
            printf("c=%d pt=%d mg=%d eg=%d\n", color, pt,
                   mg_eval[color], eg_eval[color]);
#           endif

        }
    }
#   ifdef DEBUG_EVAL
    printf("phase:%d mg[WHITE]:%d mg[BLACK]:%d eg[WHITE]:%d eg[BLACK]:%d\n",
           pos->phase, mg_eval[WHITE], mg_eval[BLACK], eg_eval[WHITE], eg_eval[BLACK]);
#   endif

    return eval[WHITE] - eval[BLACK];
}
