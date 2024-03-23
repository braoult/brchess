/* common-test.h - common static vars/funcs test
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
#include "chessdefs.h"
/* when below FENs are in a struct with selection per test */
#define NOTEST      0
#define FEN         1
#define BITBOARD    2
#define MOVEGEN     4
#define ATTACK      8
#define MOVEDO     16

struct fentest {
    int line;
    uint modules;
    char *comment;
    char *fen;
} fentest[] = {
    /*
    { __LINE__.
      ATTACK,
      "checkers: ",
      ""
    },
     */
    /* tests rank movegen bug - FIXED
     */
    //"4k3/pppppppp/8/8/8/8/PPPPPPPP/2BRK3 w - - 0 1",
    //"4k3/pppppppp/8/8/8/8/PPPPPPPP/1B1R1K2 w - - 0 1",
    { __LINE__, MOVEGEN,
      "illegal white e.p.",
      "3k4/8/8/2qpPK2/8/8/8/8 w - d6 0 1",
    },
    { __LINE__, MOVEGEN,
      "illegal black e.p.",
      "8/8/8/8/2QPpk2/8/8/3K4 b - d3 0 1",
    },

    { __LINE__, MOVEGEN,
      "illegal white e.p.",
      "3k4/8/5K2/3pP3/8/2b5/8/8 w - d6 0 1",
    },
    { __LINE__, MOVEGEN,
      "illegal black e.p.",
      "8/8/2B5/8/3Pp3/5k2/8/3K4 b - d3 0 1",
    },

    { __LINE__, MOVEGEN,
      "legal white e.p.",
      "1K1k4/8/8/3pP3/8/6b1/8/8 w - d6 0 1",
    },
    { __LINE__, MOVEGEN,
      "legal black e.p.",
      "8/8/6B1/8/3Pp3/8/8/1k1K4 b - d3 0 1",
    },

    { __LINE__, MOVEGEN,
      "white mate.",
      "1nbqkbn1/ppp1pppp/8/r1rpP1K1/8/8/PPPP1PPP/RNBQ1BNR w - d6 0 1",
    },
    { __LINE__, MOVEGEN,
      "illegal e.p.",
      "1nbqkbn1/ppp1pppp/8/r1rpP1K1/8/8/PPPP1PPP/RNBQ1BNR w - d6 0 1",
    },
    { __LINE__, ATTACK,
      "checkers: a1 h1",
      "1k6/8/8/8/8/8/8/r2K3r w - - 1 1"
    },
    { __LINE__, ATTACK,
      "checkers: a8 h8",
      "R2k3R/8/8/8/8/8/8/1K6 b - - 1 1"
    },
    { __LINE__, ATTACK,
      "checkers: b3 g3",
      "1k6/8/8/8/8/1r1K2r1/8/8 w - - 1 1"
    },

    { __LINE__, ATTACK,
      "checkers: b6 g6",
      "8/8/1R1k2R1/8/8/8/8/1K6 b - - 1 1"
    },

    { __LINE__, ATTACK,
      "checkers: g2 g7",
      "8/k5r1/8/8/6K1/8/6r1/8 w - - 1 1"
    },
    { __LINE__, ATTACK,
      "checkers: g2 g7",
      "8/6R1/8/6k1/8/8/K5R1/8 b - - 1 1"
    },
    { __LINE__, ATTACK,
      "checkers: d5 e3, pinners: none (2 pieces between attacker & K)",
      "3k4/8/8/3r3b/b7/1N2nn2/2n1B3/rNBK1Rbr w - - 1 1"
    },

    { __LINE__, ATTACK,
      "checkers: d4 e6 pinners: h4 a5 a8 h8",
      "Rn1k1r1R/4b3/1n2N3/B7/3R3B/8/8/3K4 b - - 1 1"
    },
    { __LINE__, ATTACK,
      "checkers: d5 e3, pinners: a1 h1 a4 h5",
      "3k4/8/8/3r3b/b7/1N2n3/4B3/rN1K1R1r w - - 1 0"
    },

    { __LINE__, MOVEGEN,
      "only pawn captures",
      "4k3/8/2p1p3/2PpP3/8/pp4pp/PP4PP/4K3 w - d6 0 1"
    },

    { __LINE__, FEN | ATTACK,
      "checker: h4",
      "4k3/8/8/8/7b/8/8/4K3 w - - 0 1"
    },
// First game moves
    { __LINE__, FEN | MOVEGEN,
      "initial pos",
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
    },
    { __LINE__, FEN | MOVEGEN,
      "1.e4",
      "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1"
    },
    { __LINE__, FEN | MOVEGEN,
      "1.Nh3",
      "rnbqkbnr/pppppppp/8/8/8/7N/PPPPPPPP/RNBQKB1R b KQkq - 1 1"
    },
    { __LINE__, FEN | MOVEGEN,
      "1.e4 e5 2.Nf3 Nc6",
      "r1bqkbnr/pp1ppppp/2n5/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 0 1"
    },

    // castling test
    // both can castle queen only
    { __LINE__, FEN | MOVEGEN,
      "",
      "r3k2r/8/3B4/8/8/3b4/8/R3K2R w KQkq - 0 1"
    },
    { __LINE__, FEN | MOVEGEN,
      "",
      "r3k2r/8/3BB3/8/8/3bb3/8/R3K2R w KQkq - 0 1"
    },
    { __LINE__, FEN | MOVEGEN,
      "",
      "r2bkb1r/8/8/8/8/3bb3/8/R2BKB1R w KQkq - 0 1"
    },
    //
    { __LINE__, FEN | MOVEGEN,
      "4 castle possible, only K+R",
      "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1"
    },
    //
    { __LINE__, FEN | MOVEGEN,
      "only kings on A1/A8, white to play",
      "k7/8/8/8/8/8/8/K7 w - - 0 1"
    },
    //
    { __LINE__, FEN | MOVEGEN,
      "only one move possible (Pf2xBg3)",
      "k7/8/8/1p1p4/pPpPp3/P1PpPpb1/NBNP1P2/KBB1B3 w - - 0 1"
    },
    //
    { __LINE__, FEN | MOVEGEN,
      "only 2 moves possible (Ph5xg6 e.p., Ph5-h6)",
      "k7/8/8/1p1p2pP/pPpPp3/P1PpPp2/NBNP1P2/KBB1B3 w - g6 0 1"
    },
    //
    { __LINE__, FEN | MOVEGEN,
      "2 Kings, W/B/ pawns on 7th for promotion",
      "k4n2/4P3/8/8/8/8/4p3/K4N2 w - - 0 1"
    },
    // white castled, and can e.p. on c6 black can castle
    // white is a pawn down
    // white has 36 moves: P=11 + 1 e.p. N=6+3 B=5+5 R=1 Q=3 K=1 + 1 e.p.
    // black has 33 moves: P=11 N=2+7 B=5 R=3 Q=3 K=1 + castle
    { __LINE__, FEN | MOVEGEN,
      "",
      "rnbqk2r/pp1pbpp1/7p/2pPp3/4n3/3B1N2/PPP2PPP/RNBQ1RK1 w kq c6 0 7"
    },

    { __LINE__, FEN | MOVEGEN,
      "",
      "4k3/4p3/8/b7/1BR1p2p/1Q3P2/5N2/4K3 w - - 0 1"
    },
    { __LINE__, FEN | MOVEGEN,
      "",
      "r1bq1rk1/pppp1ppp/2n2n2/4p3/2B1P3/3PPN2/PPP3PP/RN1QK2R b KQ - 1 7"
    },
    { __LINE__, FEN | MOVEGEN,
      "",
      "6k1/6pp/R2p4/p1p5/8/1P1r3P/6P1/6K1 b - - 3 3"
    },

    // some of tests below are from:
    // - Rodent IV
    // - https://www.chessprogramming.net/perfect-perft/
    { __LINE__, FEN | MOVEGEN,
      "",
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"
    },
    { __LINE__, FEN | MOVEGEN,
      "",
      "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1"
    },
    { __LINE__, FEN | MOVEGEN,
      "",
      "8/2p5/3p4/Kp5r/1R3p1k/8/4P1P1/8 w - - 0 1"
    },
    { __LINE__, FEN | MOVEGEN,
      "",
      "4rrk1/pp1n3p/3q2pQ/2p1pb2/2PP4/2P3N1/P2B2PP/4RRK1 b - - 7 19"
    },
    { __LINE__, FEN | MOVEGEN,
      "",
      "rq3rk1/ppp2ppp/1bnpb3/3N2B1/3NP3/7P/PPPQ1PP1/2KR3R w - - 7 14"
    },
    { __LINE__, FEN | MOVEGEN,
      "",
      "r1bq1r1k/1pp1n1pp/1p1p4/4p2Q/4Pp2/1BNP4/PPP2PPP/3R1RK1 w - - 2 14"
    },
    { __LINE__, FEN | MOVEGEN,
      "",
      "r3r1k1/2p2ppp/p1p1bn2/8/1q2P3/2NPQN2/PPP3PP/R4RK1 b - - 2 15"
    },
    { __LINE__, FEN | MOVEGEN,
      "",
      "1rbqk1nr/p3ppbp/2np2p1/2p5/1p2PP2/3PB1P1/PPPQ2BP/R2NK1NR b KQk - 0 1"
    },
    { __LINE__, FEN | MOVEGEN,
      "",
      "r1bqk2r/pp1p1ppp/2n1pn2/2p5/1bPP4/2NBP3/PP2NPPP/R1BQK2R b KQkq - 0 1"
    },
    { __LINE__, FEN | MOVEGEN,
      "",
      "rnb1kb1r/ppp2ppp/1q2p3/4P3/2P1Q3/5N2/PP1P1PPP/R1B1KB1R b KQkq - 0 1"
    },
    { __LINE__, FEN | MOVEGEN,
      "",
      "r1b2rk1/pp2nppp/1b2p3/3p4/3N1P2/2P2NP1/PP3PBP/R3R1K1 b - - 0 1"
    },
    { __LINE__, FEN | MOVEGEN,
      "",
      "n1q1r1k1/3b3n/p2p1bp1/P1pPp2p/2P1P3/2NBB2P/3Q1PK1/1R4N1 b - - 0 1"
    },
    { __LINE__, FEN | MOVEGEN,
      "",
      "r1bq1r1k/b1p1npp1/p2p3p/1p6/3PP3/1B2NN2/PP3PPP/R2Q1RK1 w - - 1 16"
    },
    { __LINE__, FEN | MOVEGEN,
      "",
      "3r1rk1/p5pp/bpp1pp2/8/q1PP1P2/b3P3/P2NQRPP/1R2B1K1 b - - 6 22"
    },
    { __LINE__, FEN | MOVEGEN,
      "",
      "r1q2rk1/2p1bppp/2Pp4/p6b/Q1PNp3/4B3/PP1R1PPP/2K4R w - - 2 18"
    },
    { __LINE__, FEN | MOVEGEN,
      "",
      "4k2r/1pb2ppp/1p2p3/1R1p4/3P4/2r1PN2/P4PPP/1R4K1 b - - 3 22"
    },
    { __LINE__, FEN | MOVEGEN,
      "",
      "3q2k1/pb3p1p/4pbp1/2r5/PpN2N2/1P2P2P/5PP1/Q2R2K1 b - - 4 26"
    },
    { __LINE__, FEN | MOVEGEN,
      "",
      "2r5/8/1n6/1P1p1pkp/p2P4/R1P1PKP1/8/1R6 w - - 0 1"
    },
    { __LINE__, FEN | MOVEGEN,
      "",
      "r2q1rk1/1b1nbppp/4p3/3pP3/p1pP4/PpP2N1P/1P3PP1/R1BQRNK1 b - - 0 1"
    },
    { __LINE__, FEN | MOVEGEN,
      "",
      "6k1/5pp1/7p/p1p2n1P/P4N2/6P1/1P3P1K/8 w - - 0 35"
    },
    { __LINE__, FEN | MOVEGEN,
      "",
      "r4rk1/1pp1q1pp/p2p4/3Pn3/1PP1Pp2/P7/3QB1PP/2R2RK1 b - - 0 1"
    },

    { __LINE__, FEN | MOVEGEN,
      "",
      "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"
    },
    { __LINE__, FEN | MOVEGEN,
      "",
      "1k6/1b6/8/8/7R/8/8/4K2R b K - 0 1"
    },
    { __LINE__, FEN | MOVEGEN,
      "Illegal ep move #1",
      "3k4/3p4/8/K1P4r/8/8/8/8 b - - 0 1"
    },
    { __LINE__, FEN | MOVEGEN,
      "Illegal ep move #2",
      "8/8/4k3/8/2p5/8/B2P2K1/8 w - - 0 1"
    },
    { __LINE__, FEN | MOVEGEN,
      "EP Capture Checks Opponent",
      "8/8/1k6/2b5/2pP4/8/5K2/8 b - d3 0 1"
    },
    { __LINE__, FEN | MOVEGEN,
      "Short Castling Gives Check",
      "5k2/8/8/8/8/8/8/4K2R w K - 0 1"
    },
    { __LINE__, FEN | MOVEGEN,
      "Long Castling Gives Check",
      "3k4/8/8/8/8/8/8/R3K3 w Q - 0 1"
    },
    { __LINE__, FEN | MOVEGEN,
      "Castle Rights",
      "r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1"
    },
    { __LINE__, FEN | MOVEGEN,
      "Castling Prevented",
      "r3k2r/8/3Q4/8/8/5q2/8/R3K2R b KQkq - 0 1"
    },
    { __LINE__, FEN | MOVEGEN,
      "Promote out of Check",
      "2K2r2/4P3/8/8/8/8/8/3k4 w - - 0 1"
    },
    { __LINE__, FEN | MOVEGEN,
      "Discovered Check",
      "8/8/1P2K3/8/2n5/1q6/8/5k2 b - - 0 1"
    },
    { __LINE__, FEN | MOVEGEN,
      "Promote to give check",
      "4k3/1P6/8/8/8/8/K7/8 w - - 0 1"
    },
    { __LINE__, FEN | MOVEGEN,
      "Under Promote to give check",
      "8/P1k5/K7/8/8/8/8/8 w - - 0 1"
    },
    { __LINE__, FEN | MOVEGEN,
      "Self Stalemate",
      "K1k5/8/P7/8/8/8/8/8 w - - 0 1"
    },
    { __LINE__, FEN | MOVEGEN,
      "Stalemate & Checkmate",
      "8/k1P5/8/1K6/8/8/8/8 w - - 0 1"
    },
    { __LINE__, FEN | MOVEGEN,
      "Stalemate & Checkmate",
      "8/8/2k5/5q2/5n2/8/5K2/8 b - - 0 1"
    },

    { __LINE__, FEN,
      "illegal EP and castle flags, fix-able by fen parser, SF crash",
      "4k3/8/8/8/7B/8/8/4K3 w KQkq e6 0 1"
    },
    { __LINE__, FEN,
      "illegal, SF crash",
      "4k3/8/8/8/7b/8/8/4K3 b - - 0 1"
    },
    { __LINE__, FEN,
      "illegal, SF crash",
      "2r1k3/3B4/8/8/8/8/8/4K3 w - - 0 1"
    },
    { __LINE__, FEN,
      "illegal, SF crash",
      "2r1k3/3P4/8/8/8/8/8/4K3 w - - 0 1"
    },
    { __LINE__, MOVEDO,
      "simple movedo/undo: only 2 W knights",
      "8/1k6/8/8/8/8/6K1/1NN5 w - - 0 1"
    },
    { __LINE__, MOVEDO,
      "simple movedo/undo: only 2 W knights",
      "8/1k6/8/8/8/8/6K1/1NN5 w - - 0 1"
    },
    { __LINE__, MOVEDO,
      "simple movedo/undo: only 2 W knights",
      "5n2/1k6/8/8/5K2/8/P7/1N6 w - - 0 1"
    },
    { __LINE__, 0, NULL, NULL }
};

static int cur = -1;

static char *next_fen(uint module)
{
    cur++;
    while (fentest[cur].fen && !(fentest[cur].modules & module))
        cur++;
    return fentest[cur].fen;
}

static __unused char* cur_comment()
{
    return fentest[cur].comment;
}

static __unused int cur_line()
{
    return fentest[cur].line;
}
