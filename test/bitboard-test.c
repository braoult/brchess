//#include "debug.h"
//#include "pool.h"

#include <stdio.h>
#include <string.h>

#include "bug.h"
#include "position.h"
#include "piece.h"
#include "bitboard.h"
#include "hyperbola-quintessence.h"

int main(int __unused ac, __unused char**av)
{
    char str[256];
    bitboard_init();
    hyperbola_init();
    for (int i = 0; i < 64; ++i) {
        sprintf(str, "\n%#x:\n   %-22s%-22s%-22s%-22s%-22s%-22s%-22s", i,
                "sliding", "diagonal", "antidiagonal", "file", "rank", "knight",
                "king"
                );
        bitboard_print_multi(str, 7,
                             bb_file[i] | bb_rank[i] |
                             bb_diag[i] | bb_anti[i],
                             bb_diag[i], bb_anti[i],
                             bb_file[i], bb_rank[i],
                             bb_knight[i], bb_king[i]);
    }
    bitboard_print_multi("a1-a8 a1-h8 a1-h1 a2-a7 a2-g7 a2-g2", 6,
                         bb_between[A1][A8], bb_between[A1][H8],
                         bb_between[A1][H1], bb_between[A2][A7],
                         bb_between[A2][G7], bb_between[A2][G2]);
    bitboard_print_multi("c3-c6 c3-f6 c3-f3 c3-e1 c3-c1 c3-a1 c3-a3 c3-a5", 8,
                         bb_between[C3][C6], bb_between[C3][F6],
                         bb_between[C3][F3], bb_between[C3][E1],
                         bb_between[C3][C1], bb_between[C3][A1],
                         bb_between[C3][A3], bb_between[C3][A5]);
    bitboard_print_multi("c4-c6 c4-f6 c4-f3 c4-e1 c4-c1 c4-a1 c4-a3 c4-a5", 8,
                         bb_between[C4][C6], bb_between[C4][F6],
                         bb_between[C4][F3], bb_between[C4][E1],
                         bb_between[C4][C1], bb_between[C4][A1],
                         bb_between[C4][A3], bb_between[C4][A5]);
    /*
     * for (square_t sq = 0; sq < 64; ++sq) {
     *     sprintf(str, "%2d %#lx %#lx knight", sq, bb_sq[sq], bb_knight[sq]);
     *     bitboard_print(str, bb_knight[sq]);
     *     sprintf(str, "%2d %#lx %#lx knight", sq, bb_sq[sq], bb_king[sq]);
     *     bitboard_print(str, bb_king[sq]);
     * }
     */
    return 0;
}
