#ifndef POSITION_H
#define POSITION_H

#include <stdint.h>

#include "board.h"

typedef struct position {
    short turn;
    short castle;
    short clock_50;
    short curmove;
    SQUARE en_passant;
    BOARD *board;
} POS;

void pos_print(POS *pos);
POS *pos_init(POS *pos);
POS *pos_startpos(POS *pos);
POS *pos_create();

#endif
