#ifndef PIECE_H
#define PIECE_H

#include "chessdefs.h"
#include "board.h"

typedef struct piece {
    short piece;
    short color;
    SQUARE square;
    short castle;
    float value;
} PIECE, PLAYER_PIECES[16];

#endif
