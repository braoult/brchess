/* Starting Position :
 * rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
 * After 1.e4 :
 * rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1
 * After 1... c5 :
 * rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2
 * After 2. Nf3:
 * rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2
 *
 * 1 : White uppercase
 * 2 : next move (w or b)
 * 3 : Castling capabilities: "-" if none, KQ/kq if white/black can castle
 *     on K or Q side
 * 4 : en-passant: if pawn just moved 2 squares, indicate target square (e.g.
 *     for e2-e4 this field is e3)
 * 5 : half moves since last capture or pawn advance (for 50 moves rule)
 * 6 : full moves, starts at 1, increments after black move
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "chessdefs.h"
#include "position.h"
#include "board.h"
#include "fen.h"

// warning, we expect a valid fen input
POS *fen2pos(POS *pos, char *fen)
{
    char *p = fen;
    int rank, file, skip;
    BOARD *board = pos->board;
#   define SKIP_BLANK(p) for(;*(p) == ' '; (p)++)

    //pos_init(pos);

    // 1) get piece placement information
    for (rank = 7, file = 0; *p && *p != ' '; ++p) {
        char cp = toupper(*p);
        switch (cp) {
            case CHAR_PAWN:
                board[SQ88(file, rank)]->piece = PAWN;
                goto color;
            case CHAR_KNIGHT:
                board[SQ88(file, rank)]->piece = KNIGHT;
                goto color;
            case CHAR_BISHOP:
                board[SQ88(file, rank)]->piece = BISHOP;
                goto color;
            case CHAR_ROOK:
                board[SQ88(file, rank)]->piece = ROOK;
                goto color;
            case CHAR_QUEEN:
                board[SQ88(file, rank)]->piece = QUEEN;
                goto color;
            case CHAR_KING:
                board[SQ88(file, rank)]->piece = KING;
                goto color;
                //printf("f=%d r=%d *p=%c piece=%c\n", file, rank, *p, cp);
            color:
                SET_COLOR(board[SQ88(file, rank)]->piece, islower(*p));
                //board[SQ88(file, rank)]->piece |= isupper(*p)? WHITE: BLACK;
                file++;
                break;
            case '/':
                rank--;
                file = 0;
                break;
            default:
                skip = cp - '0';
                file += skip;
                while (skip--) {
                    board[SQ88(file, rank)]->piece = EMPTY;
                }
        }
    }
    for (rank = 7; rank >= 0; --rank) {
        for (file = 0; file < 8; ++file) {
            printf("%2x ", board[SQ88(file, rank)]->piece);
        }
        putchar('\n');
    }


    // 2) next move color
    SKIP_BLANK(p);
    pos->turn = *p == 'w' ? TURN_WHITE : TURN_BLACK;
    p++;

    // 3) castle status
    SKIP_BLANK(p);
    pos->castle = 0;
    if (*p != '-') {
        for (; *p && *p != ' '; ++p) {
            switch (*p) {
                case 'K':
                    pos->castle |= CASTLE_WK;
                    break;
                case 'k':
                    pos->castle |= CASTLE_BK;
                    break;
                case 'Q':
                    pos->castle |= CASTLE_WQ;
                    break;
                case 'q':
                    pos->castle |= CASTLE_BQ;
                    break;
            }
        }
    }

    // 4) en passant
    SKIP_BLANK(p);
    //printf("pos=%d\n", (int)(p-fen));
    pos->en_passant = 0;
    if (*p != '-') {
        //printf("passant=%c\n", *p);
        SET_F(pos->en_passant, C2FILE(*p++));
        SET_R(pos->en_passant, C2RANK(*p++));
        //printf("passant=%c\n", *p);
    } else {
        p++;
    }

    // 5) half moves since last capture or pawn move and
    // 6) current move number
    SKIP_BLANK(p);
    //printf("pos=%d\n", (int)(p-fen));
    sscanf(p, "%hd %hd", &pos->clock_50, &pos->curmove);
    return pos;
}

#ifdef FENBIN
int main(int ac, char**av)
{
    POS *pos;

    pos = pos_create();
    if (ac == 1) {
        pos_startpos(pos);
    } else {
        fen2pos(pos, av[1]);
    }
    pos_print(pos);
}
#endif
