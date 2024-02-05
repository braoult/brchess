#include "debug.h"
#include "pool.h"
#include "../src/position.h"
#include "../src/fen.h"

int main(int ac, char**av)
{
    pos_t *pos;

    debug_init(5, stderr, true);
    piece_pool_init();
    pos_pool_init();
    pos = pos_get();
    if (ac == 1) {
        pos_startpos(pos);
    } else {
        fen2pos(pos, av[1]);
    }
    pos_print(pos);
}
