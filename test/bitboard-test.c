//#include "debug.h"
//#include "pool.h"

#include <stdio.h>
#include <string.h>

#include "../src/bitboard.h"
#include "../src/position.h"
#include "../src/piece.h"
//#include "../src/fen.h"

int main(int __unused ac, __unused char**av)
{
    char str[128];
    bitboard_init();
    for (int i = 0; i < 64; ++i) {
        char str[128];
        sprintf(str, "\ndiag/antidiag/vert %d", i);
        bitboard_print_multi(str, 5,
                             sq_bbfile[i] | sq_bbrank[i] | sq_bbdiag[i] | sq_bbanti[i],
                             sq_bbfile[i], sq_bbrank[i],
                             sq_bbdiag[i], sq_bbanti[i]);
    }
    for (square_t sq = 0; sq < 64; ++sq) {
        sprintf(str, "%2d %#lx %#lx knight", sq, sq_bb[sq], knight_attacks[sq]);
        bitboard_print(str, knight_attacks[sq]);
        sprintf(str, "%2d %#lx %#lx knight", sq, sq_bb[sq], king_attacks[sq]);
        bitboard_print(str, king_attacks[sq]);
    }
    return 0;
}
