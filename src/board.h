#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>

/* ffff rrrr */
typedef unsigned char SQUARE;
#define SQUARE_F(s) ((s) >> 4)
#define SQUARE_R(s) ((s) & 0x0f)
#define SET_F(s, f) ((s) &= 0x0f, (s) |= (f)<<4)
#define SET_R(s, r) ((s) &= 0xf0, (s) |= (r))

typedef struct {
    unsigned char piece;
    //struct piece *s_piece;
} BOARD[8*8*2];                                    /* 0x88 board */

/* definitions for 0x88 representation */
#define SQ88(f, r) (16 * (r) + (f))
#define FILE88(s)  ((s) & 7)
#define RANK88(s)  ((s) >> 8)

/* piece notation */
#define CHAR_EMPTY   ' '
#define CHAR_PAWN    'P'
#define CHAR_KNIGHT  'N'
#define CHAR_BISHOP  'B'
#define CHAR_ROOK    'R'
#define CHAR_QUEEN   'Q'
#define CHAR_KING    'K'

#define C2FILE(c)  (tolower(c)-'a')
#define C2RANK(c) (tolower(c)-'1')
#define FILE2C(c)  ((c)+'a')
#define RANK2C(c) ((c)+'1')

#endif
