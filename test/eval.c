#include "debug.h"

#include "../src/position.h"
#include "../src/eval.h"
#include "../src/fen.h"
#include "../src/move.h"

int main(int ac, char**av)
{
    pos_t *pos;
    eval_t res;

    debug_init(5, stderr, true);

    piece_pool_init();
    moves_pool_init();
    pos_pool_init();
    pos = pos_get();

    if (ac == 1) {
        pos_startpos(pos);
    } else {
        fen2pos(pos, av[1]);
    }

    pos_print(pos);
    pos_pieces_print(pos);

    moves_gen_all(pos);

    pos_print(pos);
    moves_print(pos, M_PR_SEPARATE);
    res = eval(pos);
    printf("eval=%d centipawns)\n", res);
}
