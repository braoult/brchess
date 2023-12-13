#include <stdio.h>
#include "debug.h"

#include "../src/fen.h"
#include "../src/move.h"

int main(int ac, char**av)
{
    pos_t *pos;

    debug_init(5, stderr, true);
    piece_pool_init();
    moves_pool_init();
    pos_pool_init();
    pos = pos_get();

    if (ac == 1) {
        fen2pos(pos, "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2");
        //pos_startpos(pos);
    } else {
        fen2pos(pos, av[1]);
    }
    //printf("turn = %d opponent = %d\n", pos->turn, OPPONENT(pos->turn));
    moves_gen_all(pos);
    pos_print(pos);
    pos_pieces_print(pos);
    moves_print(pos, M_PR_SEPARATE);

    //bitboard_print2(castle_squares[0].controlled, castle_squares[1].controlled);
    //bitboard_print2(castle_squares[0].occupied, castle_squares[1].occupied);
}
