//#include <stdio.h>

#include "bug.h"

#include "chessdefs.h"
#include "bitboard.h"
#include "position.h"
#include "fen.h"

int main(int ac, char**av)
{
    pos_t *pos;
    bitboard_t mask = A1bb | C3bb | A8bb | G7bb | H8bb | H1bb;
    const char *fen;
    char revfen[128];
    int comp;
    //debug_init(5, stderr, true);
    //pos_pool_init();
    bitboard_init();
    pos = pos_new();
    if (ac == 1) {
        fen = startfen;
        startpos(pos);
    } else {
        fen = av[1];
        fen2pos(pos, fen);
    }
    pos_print(pos);
    pos_print_mask(pos, mask);
    printf("ULL=#%lx %#lx %#lx %#lx #%lx\n", A5bb, H5bb, H6bb, H7bb, H8bb);
    printf("ULL=%llx %llx %llx\n", mask(A5),  mask(H7), mask(H8));

    pos2fen(pos, revfen);
    //printf("reverse fen=[%s]\n", pos2fen(pos, NULL));
    comp = strcmp(fen, revfen);
    printf("compare=%d - %s\n", comp, comp? "NOK": "OK");
    pos_print_board_raw(pos, 0);
    pos_print_board_raw(pos, 1);
}
