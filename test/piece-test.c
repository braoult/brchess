#include <stdio.h>

#include "brlib.h"

#include "piece.h"

int main(__unused int ac, __unused char**av)
{

    piece_t p;
    char *test="PNBRQKpnbrqk";

    for (u64 i = 0; i < sizeof(test); ++i) {
        char c1 = test[i], *c2;
        p = piece_from_fen(c1);
        c2 = piece_to_fen(p);
        printf("c1=%c c2=%c\n", c1, *c2);
    }
    return 0;
}
