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
    for (square_t sq = 0; sq < 64; ++sq) {
        sprintf(str, "%2d %#lx %#lx knight", sq, sq_bb[sq], knight_attacks[sq]);
        bitboard_print(knight_attacks[sq], str);
        sprintf(str, "%2d %#lx %#lx knight", sq, sq_bb[sq], king_attacks[sq]);
        bitboard_print(king_attacks[sq], str);
    }
    return 0;
}
