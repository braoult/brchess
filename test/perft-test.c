/* perft-test.c - perft test.
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
#include <time.h>
#include <locale.h>

#include "chessdefs.h"
#include "fen.h"
#include "position.h"
#include "move.h"
#include "move-do.h"
#include "move-gen.h"
#include "search.h"

#include "common-test.h"

#define RD 0
#define WR 1

typedef struct {
    move_t move;
    int count;
} ddperft;

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

static u64 send_stockfish_fen(FILE *desc, pos_t *pos, movelist_t *movelist,
                               char *fen, int depth)
{
    char *buf = NULL;
    u64 count, mycount = 0, fishcount;
    size_t alloc = 0;
    ssize_t buflen;

    pos_clear(pos);

    move_t *moves = movelist->move;
    int *nmoves = &movelist->nmoves;
    *nmoves = 0;
    //char nodescount[] = "Nodes searched";
    //printf("nmoves = %d\n", nmoves);
    fflush(stdout);
    //sprintf(str, "stockfish \"position fen %s\ngo perft depth\n\"", fen);
    fprintf(desc, "position fen %s\ngo perft %d\n", fen, depth);
    //fflush(desc);
    while ((buflen = getline(&buf, &alloc, stdin)) > 0) {
        buf[--buflen] = 0;
        if (buflen == 0)
            continue;
        if (sscanf(buf, "Nodes searched: %lu", &fishcount) == 1) {
            break;
        }
        //printf("%d: %s\n", line++, buf);
        if (sscanf(buf, "%*4s: %lu", &count) == 1) {
            square_t from = sq_from_string(buf);
            square_t to   = sq_from_string(buf + 2);
            mycount += count;
            //printf("move found: %c%c->%c%c %s->%s count=%d\n",
            //       buf[0], buf[1], buf[2], buf[3],
            //       sq_to_string(from), sq_to_string(to),
            //       count);
            moves[(*nmoves)++] = move_make(from, to);

        } else if (sscanf(buf, "%*5s: %lu", &count) == 1) {
            square_t from = sq_from_string(buf);
            square_t to   = sq_from_string(buf + 2);
            piece_type_t promoted = piece_t_from_char(*(buf + 4));
            mycount += count;
            //printf("move found: %c%c->%c%c %s->%s count=%d\n",
            //       buf[0], buf[1], buf[2], buf[3],
            //       sq_to_string(from), sq_to_string(to),
            //       count);
            moves[(*nmoves)++] = move_make_promote(from, to, promoted);
        }
    }
    //pos->moves.nmoves = nmoves;
    // printf("fishcount=%d mycount=%d\n", fishcount, mycount);
    free(buf);
    return mycount;
}

static __unused bool movelists_equal(movelist_t *fish, movelist_t *me)
{
    move_t *m1 = fish->move, *m2 = me->move;
    int n1 = fish->nmoves, n2 = me->nmoves;
    int mask = 077777;

    if (n1 != n2)
        return false;
    for (int cur = 0; cur < n1; ++cur) {
        if ((m1[cur] & mask) != (m2[cur] & mask))
            return false;
    }
    return true;
}

static __unused void compare_moves(movelist_t *fish, movelist_t *me)
{
    char str1[1024] = {0}, str2[1024] = {0}, tmpstr[1024];
    char *skip = "      ";
    move_t *m1 = fish->move;
    move_t *m2 = me->move;
    int n1 = fish->nmoves;
    int n2 = me->nmoves;

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

static int usage(char *prg)
{
    fprintf(stderr, "Usage: %s [-d depth] [-p pertf-modules] -n\n", prg);
    return 1;
}

int main(int __unused ac, __unused char**av)
{
    int i = 0, test_line;
    u64 sf_count = 0, my_count;
    char *fen;
    pos_t *pos, *savepos, *fishpos = pos_new();
    movelist_t fishmoves;
    //move_t move;
    FILE *outfd = NULL;
    s64 ms1 = 0, ms1_total = 0;
    s64 ms2 = 0, ms2_total = 0;

    int opt, depth = 6, run = 3;
    bool sf_run = true;

    while ((opt = getopt(ac, av, "nd:p:")) != -1) {
        switch (opt) {
            case 'd':
                depth = atoi(optarg);
                break;
            case 'p': /* 1 or 2 */
                run = atoi(optarg);
                break;
            case 'n':
                sf_run = false;
                break;
            default:
                    return usage(*av);
        }
    }
    //if (ac > 1)
    //    depth = atoi(av[1]);
    //if (ac > 2)
    //    run =  atoi(av[2]) & 3;
    printf("depth = %d run = %x sf = %s\n", depth, run, sf_run? "true": "false");

    if (!run)
        exit(0);
    setlocale(LC_NUMERIC, "");
    setlinebuf(stdout);                           /* line-buffered stdout */

    if (sf_run)
        outfd = open_stockfish();

    bitboard_init();
    hyperbola_init();

    CLOCK_DEFINE(clock, CLOCK_PROCESS);
    while ((fen = next_fen(PERFT | MOVEDO))) {
        test_line = cur_line();
        if (!(pos = fen2pos(NULL, fen))) {
            printf("wrong fen %d: [%s]\n", i, fen);
            continue;
        }
        if (sf_run)
            sf_count = send_stockfish_fen(outfd, fishpos, &fishmoves, fen, depth);

        savepos = pos_dup(pos);

        if (run & 1) {
            clock_start(&clock);
            my_count = perft(pos, depth, 1);
            ms1 = clock_elapsed_ms(&clock);
            ms1_total += ms1;

            if (!sf_run || sf_count == my_count) {
                printf("pt1 OK : line=%3d perft=%'lu %'ldms lps=%'lu \"%s\"\n",
                       test_line, my_count, ms1,
                       ms1? my_count*1000l/ms1: 0,
                       fen);
            } else  {
                printf("pt1 ERR: line=%3d sf=%'lu me=%'lu \"%s\"\n",
                       test_line, sf_count, my_count, fen);
            }
        }

        if (run & 2) {
            clock_start(&clock);
            my_count = perft_new_pinners(pos, depth, 1);
            ms2 = clock_elapsed_ms(&clock);
            ms2_total += ms2;

            if (!sf_run || sf_count == my_count) {
                printf("pt2 OK : line=%3d perft=%'lu %'ldms lps=%'lu \"%s\"\n",
                       test_line, my_count, ms2,
                       ms2? my_count*1000l/ms2: 0,
                       fen);
            } else  {
                printf("pt2 ERR: line=%3d sf=%'lu me=%'lu \"%s\"\n",
                       test_line, sf_count, my_count, fen);
            }
        }
        printf("\n");
        pos_del(savepos);
        pos_del(pos);
        i++;
        /* to run first test only */
        // exit(0);
    }
    if (run & 1)
        printf("total perft  %'ldms\n", ms1_total);
    if (run & 2)
        printf("total perft2 %'ldms\n", ms2_total);
    return 0;
}
