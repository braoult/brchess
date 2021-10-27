#ifndef CHESSDEFS_H
#define CHESSDEFS_H

/* We use the following notation
 * Bit  Binary          Piece/color
 *
 * 0    0000 0001       White
 * 1    0000 0010       Black
 *
 * 2    0000 0100       Pawn
 * 3    0000 1000       Knight
 * 4    0001 0000       Bishop
 * 5    0010 0000       Rook
 * 6    0100 0000       Queen
 * 7    1000 0000       King
 */

//#define INVALID  -1                               /* unused in 0x88 */
#define EMPTY        0

#define WHITE        (0)                          /* 0000000 0 */
#define BLACK        (1)                          /* 0000000 1 */

#define PAWN         (1 << 1)                     /* 0000001 0 */
#define KNIGHT       (1 << 2)                     /* 0000010 0 */
#define BISHOP       (1 << 3)                     /* 0000100 0 */
#define ROOK         (1 << 4)                     /* 0001000 0 */
#define QUEEN        (1 << 5)                     /* 0010000 0 */
#define KING         (1 << 6)                     /* 0100000 0 */

#define MASK_COLOR   0x01                         /* 0000000 1 */
#define MASK_PIECE   0x7E                         /* 0111111 0 */

#define COLOR(p)    ((p) & MASK_COLOR)
#define PIECE(p)    ((p) & MASK_PIECE)

#define IS_WHITE(p)  (!COLOR(p))
#define IS_BLACK(p)  (COLOR(p))

#define SET_WHITE(p)    ((p) &= ~MASK_COLOR)
#define SET_BLACK(p)    ((p) |= MASK_COLOR)
#define SET_COLOR(p, c) (!(c)? SET_WHITE(p): SET_BLACK(p))

#define TURN_WHITE   0
#define TURN_BLACK   1

typedef unsigned char piece_t, color_t;

#define CASTLE_WK    0x01
#define CASTLE_WQ    0x02
#define CASTLE_BK    0x04
#define CASTLE_BQ    0x08
#define CASTLE_W     0x03                         /* white castle mask */
#define CASTLE_B     0x0C                         /* black castle mask */

#endif
