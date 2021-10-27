#ifndef ROOK_H
#define ROOK_H

#include "chessdefs.h"
#include "position.h"

typedef struct {
    PIECE piece;
    SQUARE from;
    SQUARE to;

} MOVE;

extern MOVE *moves_rook(POS *pos);

#endif
