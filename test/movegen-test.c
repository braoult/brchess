//#include "debug.h"
//#include "pool.h"

#include <stdio.h>
#include <string.h>

#include "../src/position.h"
#include "../src/piece.h"
#include "../src/bitboard.h"
#include "../src/hyperbola-quintessence.h"

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
                             bb_diagonal[i] | bb_antidiagonal[i],
                             bb_diagonal[i], bb_antidiagonal[i],
                             bb_file[i], bb_rank[i],
                             bb_knight[i], bb_king[i]);
    }
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
