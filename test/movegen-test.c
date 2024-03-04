/* movegen-test.c - basic movegen tests.
 *
 * Copyright (C) 2024 Bruno Raoult ("br")
 * Licensed under the GNU General Public License v3.0 or later.
 * Some rights reserved. See COPYING.
 *
 * You should have received a copy of the GNU General Public License along with this
 * program. If not, see <https://www.gnu.org/licenses/gpl-3.0-standalone.html>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later <https://spdx.org/licenses/GPL-3.0-or-later.html>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "chessdefs.h"
#include "fen.h"
#include "move-gen.h"
#include "position.h"

#include "common-test.h"

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
    pos_t *pos, *fishpos = pos_new();
    char *fen;
    //bitboard_t wrong = 0x5088000040, tmp, loop;
    //bit_for_each64(loop, tmp, )

    bitboard_init();
    hyperbola_init();
    outfd = open_stockfish();

    while ((fen = next_fen(MOVEGEN))) {
        //printf(">>>>> %s\n", test[i]);
        printf("original fen %d: [%s]\n", i, fen);
        if (!(pos = fen2pos(NULL, fen))) {
            printf("wrong fen %d: [%s]\n", i, fen);
            continue;
        }
        pos_print(pos);
        fflush(stdout);

        /* print movelists */
        send_stockfish_fen(outfd, fishpos, fen);
        gen_all_pseudomoves(pos);
        //printf("Fu ");
        //moves_print(fishpos, 0);
        //fflush(stdout);
        //printf("Mu ");
        //moves_print(pos, 0);
        //fflush(stdout);

        /* sort and print movelists */
        move_sort_by_sq(fishpos);
        move_sort_by_sq(pos);
        // printf("\nFs ");
        // moves_print(fishpos, 0);
        // fflush(stdout);
        // printf("Ms ");
        // moves_print(pos, 0);
        // fflush(stdout);

        /* compare movelists */
        printf("\n");
        compare_moves(fishpos, pos);
        //pos_print_board_raw(pos, 1);
        //printf("%s\n", pos2fen(pos, str));
        //get_stockfish_moves(test[i]);
        //exit(0);
        pos_del(pos);
        i++;
    }
    fclose(outfd);
    return 0;
}
