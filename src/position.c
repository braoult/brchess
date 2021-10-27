#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "chessdefs.h"
#include "position.h"
#include "fen.h"

inline static char piece2char(unsigned char p)
{
    piece_t piece = PIECE(p);
    char res;

    //printf("%#x p=%#x\n", p, PIECE(p));
    switch (piece) {
        case PAWN:
            res = CHAR_PAWN;
            break;
        case KNIGHT:
            res = CHAR_KNIGHT;
            break;
        case BISHOP:
            res = CHAR_BISHOP;
            break;
        case ROOK:
            res = CHAR_ROOK;
            break;
        case QUEEN:
            res = CHAR_QUEEN;
            break;
        case KING:
            res = CHAR_KING;
            break;
        default:
            res = CHAR_EMPTY;
    }
    if (IS_BLACK(p))
        res = tolower(res);
    return res;

}

void pos_print(POS *pos)
{
    int rank, file;
    piece_t piece;
    BOARD *board = pos->board;

    printf("  +---+---+---+---+---+---+---+---+\n");
    for (rank = 7; rank >= 0; --rank) {
        printf("%c |", rank + '1');
        for (file = 0; file < 8; ++file) {
            piece = board[SQ88(file, rank)]->piece;
            printf(" %c |", piece2char(piece));
        }
        printf("\n  +---+---+---+---+---+---+---+---+\n");
    }
    printf("    A   B   C   D   E   F   G   H\n\n");
    printf("Next move: %s.\n", IS_WHITE(pos->turn) ? "white" : "black");
    printf("Possible en-passant: [%#x] ", pos->en_passant);
    if (pos->en_passant == 0)
        printf("None.\n");
    else
        printf("%d %d = %c%c\n", SQUARE_F(pos->en_passant),
               SQUARE_R(pos->en_passant),
               FILE2C(SQUARE_F(pos->en_passant)),
               RANK2C(SQUARE_R(pos->en_passant)));

    printf("castle [%#x] : ", pos->castle);

    if (pos->castle & CASTLE_WK)
        printf("K");
    if (pos->castle & CASTLE_WQ)
        printf("Q");
    if (pos->castle & CASTLE_BK)
        printf("k");
    if (pos->castle & CASTLE_BQ)
        printf("q");

    printf("\n50 half-moves-rule = %d\n", pos->clock_50);
    printf("Current move = %d\n", pos->curmove);
}

POS *pos_init(POS *pos)
{
    int file, rank;
    BOARD *board = pos->board;

    for (rank = 0; rank < 8; ++rank) {
        for (file = 0; file < 8; ++file) {
            printf("file = %d rank = %d SQ88 = %#x\n", file, rank, SQ88(file, rank));
            board[SQ88(file, rank)]->piece = EMPTY;
        }
    }

    pos->turn = TURN_WHITE;
    pos->castle = 0;
    pos->clock_50 = 0;
    pos->curmove = 0;
    pos->en_passant = 0;
    pos->en_passant = 0;

    return pos;
}

POS *pos_startpos(POS *pos)
{
    static char *startfen="rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    return fen2pos(pos, startfen);
}

POS *pos_create()
{
    POS *pos = malloc(sizeof(POS));
    if (pos) {
        pos->board = malloc(sizeof (BOARD));
        if (pos->board)
            pos_init(pos);
        else {
            free(pos);
            pos = NULL;
        }
    }
    return pos;
}
