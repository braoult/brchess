#include <stdio.h>

#include "debug.h"

#include "../src/fen.h"
#include "../src/position.h"
#include "../src/bitboard.h"

int main(int ac, char**av)
{
    pos_t *pos;
    printf("zobi\n");fflush(stdout);
    debug_init(6, stderr, true);
    log_f(5, "kfsjdhg\n");
    pos_pool_init();
    pos = pos_get();
    piece_pool_init();

    if (ac == 1) {
    printf("zoba\n");fflush(stdout);
        pos_startpos(pos);
    } else {
        fen2pos(pos, av[1]);
    }
    pos_print(pos);
    pos_pieces_print(pos);

    printf("0x1c = 11100 = C1-E1:\n");
    bitboard_print(0x1c);

    printf("0x70 = 111 = A1-C1\n");
    bitboard_print(0x70);

    printf("0x0e = 1110 = B1-D1\n");
    bitboard_print(0x0e);

    printf("0x60 = 1100000 = F1-G1\n");
    bitboard_print(0x60);

    printf("A1:\n");
    bitboard_print(A1);

    printf("1:\n");
    bitboard_print(1L);
    printf("H1:\n");
    bitboard_print(H1);
    printf("C1:\n");
    bitboard_print(C1);
    printf("D1:\n");
    bitboard_print(D1);
    printf("C1|D1:\n");
    bitboard_print(C1|D1);
    printf("H8:\n");
    bitboard_print(H8);
}
