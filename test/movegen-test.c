//#include "debug.h"
//#include "pool.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "position.h"
#include "piece.h"
#include "bitboard.h"
#include "fen.h"
#include "move.h"
#include "movegen.h"

static char *fentest[] = {
    /* tests rank movegen bug - FIXED */
    // "4k3/pppppppp/8/8/8/8/PPPPPPPP/2BRK3 w - - 0 1",
    // "4k3/pppppppp/8/8/8/8/PPPPPPPP/1B1R1K2 w - - 0 1",

    // illegal positions (en-prise king)
    "4k3/8/8/8/7b/8/8/4K3 b - - 0 1",
    "2r1k3/3P4/8/8/8/8/8/4K3 w - - 0 1",

    // initial pos
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
    // position after 1.e4
    "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1",
    // position after 1.Nh3
    "rnbqkbnr/pppppppp/8/8/8/7N/PPPPPPPP/RNBQKB1R b KQkq - 1 1",
    // after e4 e5 Nf3 Nc6
    "r1bqkbnr/pp1ppppp/2n5/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 0 1",

    //
    "4k3/4p3/8/b7/1BR1p2p/1Q3P2/5N2/4K3 w - - 0 1",
    "r1bq1rk1/pppp1ppp/2n2n2/4p3/2B1P3/3PPN2/PPP3PP/RN1QK2R b KQ - 1 7",
    "6k1/6pp/R2p4/p1p5/8/1P1r3P/6P1/6K1 b - - 3 3",
    // both can castle queen only
    "r3k2r/8/3B4/8/8/3b4/8/R3K2R w KQkq - 0 1",
    "r3k2r/8/3BB3/8/8/3bb3/8/R3K2R w KQkq - 0 1",
    "r2bkb1r/8/8/8/8/3bb3/8/R2BKB1R w KQkq - 0 1",
    // 4 castle possible, only K+R
    "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1",
    // only kings on A1/A8, white to play
    "k7/8/8/8/8/8/8/K7 w - - 0 1",
    // only one move possible (Pf2xBg3)
    "k7/8/8/1p1p4/pPpPp3/P1PpPpb1/NBNP1P2/KBB1B3 w - - 0 1",
    // only 2 moves possible (Ph5xg6 e.p., Ph5-h6)
    "k7/8/8/1p1p2pP/pPpPp3/P1PpPp2/NBNP1P2/KBB1B3 w - g6 0 1",
    // 2 Kings, W/B/ pawns on 7th for promotion
    "k4n2/4P3/8/8/8/8/4p3/K4N2 w - - 0 1",
    // white castled, and can e.p. on c6 black can castle
    // white is a pawn down
    // white has 36 moves: P=11 + 1 e.p. N=6+3 B=5+5 R=1 Q=3 K=1 + 1 e.p.
    // black has 33 moves: P=11 N=2+7 B=5 R=3 Q=3 K=1 + castle
    "rnbqk2r/pp1pbpp1/7p/2pPp3/4n3/3B1N2/PPP2PPP/RNBQ1RK1 w kq c6 0 7",

    // below tests are from:
    // - Rodent IV
    // - https://www.chessprogramming.net/perfect-perft/
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
    "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
    "4rrk1/pp1n3p/3q2pQ/2p1pb2/2PP4/2P3N1/P2B2PP/4RRK1 b - - 7 19",
    "rq3rk1/ppp2ppp/1bnpb3/3N2B1/3NP3/7P/PPPQ1PP1/2KR3R w - - 7 14",
    "r1bq1r1k/1pp1n1pp/1p1p4/4p2Q/4Pp2/1BNP4/PPP2PPP/3R1RK1 w - - 2 14",
    "r3r1k1/2p2ppp/p1p1bn2/8/1q2P3/2NPQN2/PPP3PP/R4RK1 b - - 2 15",
    "1rbqk1nr/p3ppbp/2np2p1/2p5/1p2PP2/3PB1P1/PPPQ2BP/R2NK1NR b KQk - 0 1",
    "r1bqk2r/pp1p1ppp/2n1pn2/2p5/1bPP4/2NBP3/PP2NPPP/R1BQK2R b KQkq - 0 1",
    "rnb1kb1r/ppp2ppp/1q2p3/4P3/2P1Q3/5N2/PP1P1PPP/R1B1KB1R b KQkq - 0 1",
    "r1b2rk1/pp2nppp/1b2p3/3p4/3N1P2/2P2NP1/PP3PBP/R3R1K1 b - - 0 1",
    "n1q1r1k1/3b3n/p2p1bp1/P1pPp2p/2P1P3/2NBB2P/3Q1PK1/1R4N1 b - - 0 1",
    "r1bq1r1k/b1p1npp1/p2p3p/1p6/3PP3/1B2NN2/PP3PPP/R2Q1RK1 w - - 1 16",
    "3r1rk1/p5pp/bpp1pp2/8/q1PP1P2/b3P3/P2NQRPP/1R2B1K1 b - - 6 22",
    "r1q2rk1/2p1bppp/2Pp4/p6b/Q1PNp3/4B3/PP1R1PPP/2K4R w - - 2 18",
    "4k2r/1pb2ppp/1p2p3/1R1p4/3P4/2r1PN2/P4PPP/1R4K1 b - - 3 22",
    "3q2k1/pb3p1p/4pbp1/2r5/PpN2N2/1P2P2P/5PP1/Q2R2K1 b - - 4 26",
    "2r5/8/1n6/1P1p1pkp/p2P4/R1P1PKP1/8/1R6 w - - 0 1",
    "r2q1rk1/1b1nbppp/4p3/3pP3/p1pP4/PpP2N1P/1P3PP1/R1BQRNK1 b - - 0 1",
    "6k1/5pp1/7p/p1p2n1P/P4N2/6P1/1P3P1K/8 w - - 0 35",
    "r4rk1/1pp1q1pp/p2p4/3Pn3/1PP1Pp2/P7/3QB1PP/2R2RK1 b - - 0 1",

    "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
    "1k6/1b6/8/8/7R/8/8/4K2R b K - 0 1",
    "3k4/3p4/8/K1P4r/8/8/8/8 b - - 0 1",          // Illegal ep move #1
    "8/8/4k3/8/2p5/8/B2P2K1/8 w - - 0 1",         // Illegal ep move #2
    "8/8/1k6/2b5/2pP4/8/5K2/8 b - d3 0 1",        // EP Capture Checks Opponent
    "5k2/8/8/8/8/8/8/4K2R w K - 0 1",             // Short Castling Gives Check
    "3k4/8/8/8/8/8/8/R3K3 w Q - 0 1",             // Long Castling Gives Check
    "r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1",  // Castle Rights
    "r3k2r/8/3Q4/8/8/5q2/8/R3K2R b KQkq - 0 1",   // Castling Prevented
    "2K2r2/4P3/8/8/8/8/8/3k4 w - - 0 1",          // Promote out of Check
    "8/8/1P2K3/8/2n5/1q6/8/5k2 b - - 0 1",        // Discovered Check
    "4k3/1P6/8/8/8/8/K7/8 w - - 0 1",             // Promote to give check
    "8/P1k5/K7/8/8/8/8/8 w - - 0 1",              // Under Promote to give check
    "K1k5/8/P7/8/8/8/8/8 w - - 0 1",              // Self Stalemate
    "8/k1P5/8/1K6/8/8/8/8 w - - 0 1",             // Stalemate & Checkmate
    "8/8/2k5/5q2/5n2/8/5K2/8 b - - 0 1",          // Stalemate & Checkmate

    NULL
};

#define RD 0
#define WR 1

/**
 * return write pipd fd
 */
static FILE *open_stockfish()
{
    int rpipe[2], wpipe[2];
    FILE *out_desc;
    pid_t pid;

    if ((pipe(rpipe) < 0) || (pipe(wpipe) < 0)) {
        perror("pipe");
        return NULL;
    }
    if ((pid = fork()) < 0) {
        perror("fork");
        return NULL;
    }

    if (!pid) {                                   /* stockfish */
        setvbuf(stdin,  NULL, _IOLBF, 0);
        setvbuf(stdout, NULL, _IOLBF, 0);
        setvbuf(stderr, NULL, _IOLBF, 0);

        dup2(wpipe[RD], STDIN_FILENO);
        dup2(rpipe[WR], STDOUT_FILENO);
        dup2(rpipe[WR], STDERR_FILENO);

        close(wpipe[RD]);
        close(wpipe[WR]);
        close(rpipe[RD]);
        close(rpipe[WR]);
        if (execlp("stockfish", "stockfish", NULL) == -1) {
            perror("execlp");
            return NULL;
        }
        return 0;                                 /* not reached */
    }
    /* us */
    dup2(rpipe[RD], STDIN_FILENO);
    setvbuf(stdin, NULL, _IOLBF, 0);

    close(wpipe[RD]);
    close(rpipe[RD]);
    close(rpipe[WR]);

    out_desc = fdopen(wpipe[WR], "w");
    setvbuf(out_desc, NULL, _IOLBF, 0);

    return out_desc;
}

static void send_stockfish_fen(FILE *desc, pos_t *pos, char *fen)
{
    char *buf = NULL;
    int count, __unused mycount = 0, fishcount;
    size_t alloc = 0;
    ssize_t buflen;
    pos_clear(pos);
    move_t *moves = pos->moves.move;
    int nmoves = pos->moves.nmoves;
    //char nodescount[] = "Nodes searched";
    printf("nmoves = %d\n", nmoves);
    fflush(stdout);
    //sprintf(str, "stockfish \"position fen %s\ngo perft depth\n\"", fen);
    fprintf(desc, "position fen %s\ngo perft 1\n", fen);
    //fflush(desc);
    while ((buflen = getline(&buf, &alloc, stdin)) > 0) {
        buf[--buflen] = 0;
        if (buflen == 0)
            continue;
        if (sscanf(buf, "Nodes searched: %d", &fishcount) == 1) {
            break;
        }
        //printf("%d: %s\n", line++, buf);
        if (sscanf(buf, "%*4s: %d", &count) == 1) {
            square_t from = sq_from_string(buf);
            square_t to   = sq_from_string(buf + 2);
            mycount += count;
            //printf("move found: %c%c->%c%c %s->%s count=%d\n",
            //       buf[0], buf[1], buf[2], buf[3],
            //       sq_to_string(from), sq_to_string(to),
            //       count);
            moves[nmoves++] = move_make(from, to);

        }

    }
    pos->moves.nmoves = nmoves;
    // printf("fishcount=%d mycount=%d\n", fishcount, mycount);
    free(buf);
}

static void compare_moves(pos_t *fish, pos_t *me)
{
    char str1[1024] = {0}, str2[1024] = {0}, tmpstr[1024];
    char *skip = "      ";
    move_t *m1 = fish->moves.move;
    move_t *m2 = me->moves.move;
    int n1 = fish->moves.nmoves;
    int n2 = me->moves.nmoves;

#define f(c) move_from(c)
#define t(c) move_to(c)

    for (move_t *c1 = m1, *c2 = m2; (c1 - m1 < n1) || (c2 - m2 < n2);) {
        // square_t f1 = move_from(*c1); square_t t1 = move_to(*c1);
        // square_t f2 = move_from(*c2); square_t t2 = move_to(*c2);

        /* no more move in c2 */
        if (c2 - m2 >= n2) {
            while (c1 - m1 < n1) {
                sprintf(tmpstr, " %s-%s", sq_to_string(f(*c1)), sq_to_string(t(*c1)));
                strcat (str1, tmpstr);
                c1++;
            }
            break;
        }
        if (c1 - m1 >= n1) {
            while (c2 - m2 < n2) {
                sprintf(tmpstr, " %s-%s", sq_to_string(f(*c2)), sq_to_string(t(*c2)));
                strcat (str2, tmpstr);
                c2++;
            }
            break;
        }

        /* missing move in c2 */
        if (f(*c1) < f(*c2) ||
            (f(*c1) == f(*c2) && t(*c1) < t(*c2))) {
            strcat(str2, skip);
            sprintf(tmpstr, " %s-%s", sq_to_string(f(*c1)), sq_to_string(t(*c1)));
            strcat (str1, tmpstr);
            while ((c1 - m1 < n1) && (f(*c1) < f(*c2) ||
                                      (f(*c1) == f(*c2) && t(*c1) < t(*c2)))) {
                c1++;
            }
            continue;
        }
        /* missing move in c1 */
        if (f(*c1) > f(*c2) ||
            (f(*c1) == f(*c2) && t(*c1) > t(*c2))) {
            strcat(str1, skip);
            sprintf(tmpstr, " %s-%s", sq_to_string(f(*c2)), sq_to_string(t(*c2)));
            strcat (str2, tmpstr);
            while ((c2 - m2 < n2) && (f(*c1) > f(*c2) ||
                                      (f(*c1) == f(*c2) && t(*c1) > t(*c2)))) {
                c2++;
            }
            continue;
        }
        sprintf(tmpstr, " %s-%s", sq_to_string(f(*c1)), sq_to_string(t(*c1)));
        strcat(str1, tmpstr);
        strcat(str2, tmpstr);
        c1++, c2++;
    }
    printf("F(%2d): %s\nM(%2d): %s\n", n1, str1, n2, str2);
}

int main(int __unused ac, __unused char**av)
{
    int i = 0;
    FILE *outfd;
    pos_t *mypos = pos_new(), *fishpos = pos_new();

    //bitboard_t wrong = 0x5088000040, tmp, loop;
    //bit_for_each64(loop, tmp, )

    bitboard_init();
    hyperbola_init();
    outfd = open_stockfish();

    while (fentest[i]) {
        //printf(">>>>> %s\n", test[i]);
        fen2pos(mypos, fentest[i]);
        pos_print(mypos);

        send_stockfish_fen(outfd, fishpos, fentest[i]);
        printf("Fu ");
        moves_print(fishpos, 0);
        fflush(stdout);
        gen_all_pseudomoves(mypos);
        printf("Mu ");
        moves_print(mypos, 0);
        fflush(stdout);
        puts("zobi");
        fflush(stdout);
        move_sort_by_sq(fishpos);
        printf("\nFs ");
        moves_print(fishpos, 0);
        fflush(stdout);

        move_sort_by_sq(mypos);
        printf("Ms ");
        moves_print(mypos, 0);
        fflush(stdout);
        printf("\n");
        compare_moves(fishpos, mypos);
        //pos_print_board_raw(pos, 1);
        //printf("%s\n", pos2fen(pos, str));
        //get_stockfish_moves(test[i]);
        //exit(0);
        i++;
    }
    fclose(outfd);
    return 0;
}
