#include "debug.h"
#include "pool.h"

#include "../src/position.h"
#include "../src/fen.h"

int main(int ac, char**av)
{
    pos_t *pos;


    debug_init(5, stderr, true);
    pos_pool_init();
    pos = pos_new();
    if (ac == 1) {
        startpos(pos);
    } else {
        fen2pos(pos, av[1]);
    }
    pos_print(pos);
    printf("fen=[%s]\n", pos2fen(pos, NULL));
}
