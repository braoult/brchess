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
#include <limits.h>

#include "chessdefs.h"
#include "util.h"
#include "fen.h"
#include "position.h"
#include "move.h"
#include "move-do.h"
#include "move-gen.h"
#include "perft.h"

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
    char *buf = NULL;
    size_t alloc = 0;
    ssize_t buflen;

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

    fprintf(out_desc, "uci\n");
    while (true) {
        if ((buflen = getline(&buf, &alloc, stdin)) < 0) {
            perror("getline");
            exit(1);
        }
        if (!strncmp(buf, "uciok", 5))
            break;
    }
    free(buf);
    return out_desc;
}

static void stockfish_fen(FILE *desc, char *fen)
{
    char *buf = NULL;
    size_t alloc = 0;
    ssize_t buflen;

    fprintf(desc, "ucinewgame\nisready\n");
    while ((buflen = getline(&buf, &alloc, stdin)) > 0) {
        if (!strncmp(buf, "readyok", 7))
            break;
    }
    fprintf(desc, "position fen %s\n", fen);
    free(buf);
}

static u64 stockfish_perft(FILE *desc, pos_t *pos, movelist_t *movelist,
                           int depth, int divide)
{
    char *buf = NULL, movestr[8];
    u64 count, mycount = 0, fishcount;
    size_t alloc = 0;
    ssize_t buflen;

    pos_clear(pos);

    move_t *moves = movelist->move;
    int *nmoves = &movelist->nmoves;
    *nmoves = 0;
    //char nodescount[] = "Nodes searched";
    //printf("nmoves = %d\n", nmoves);
    //fflush(stdout);
    //sprintf(str, "stockfish \"position fen %s\ngo perft depth\n\"", fen);
    //fprintf(desc, "position fen %s\ngo perft %d\n", fen, depth);
    //fflush(desc);
    fprintf(desc, "go perft %d\n", depth);

    while ((buflen = getline(&buf, &alloc, stdin)) > 0) {
        buf[--buflen] = 0;
        if (buflen == 0)
            continue;
        if (sscanf(buf, "Nodes searched: %lu", &fishcount) == 1) {
            break;
        }
        //printf("%d: %s\n", line++, buf);
        if (sscanf(buf, "%6[a-z1-8]: %lu", movestr, &count) == 2) {
            //printf("read:%s movestr:%s count:%lu\n", buf, movestr, count);
            moves[(*nmoves)++] = move_from_str(movestr);
            mycount += count;
            if (divide) {
                printf("%s: %lu\n", movestr, count);
            }
        }
    }
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
    fprintf(stderr, "Usage: %s [-cms][-d depth] [-p version] [-t size:\n", prg);
    fprintf(stderr, "\t-c:         do *not* print FEN comments\n");
    fprintf(stderr, "\t-d depth:   perft depth (default: 6)\n");
    fprintf(stderr, "\t-l line:    start from 'line' test\n");
    fprintf(stderr, "\t-m:         print moves details\n");
    fprintf(stderr, "\t-n number:  do 'number' tests (default: all)\n");
    fprintf(stderr, "\t-s:         use Stockfish to validate perft result\n");
    fprintf(stderr, "\t-t size:    Transposition Table size (Mb). Default: 32\n");
    fprintf(stderr,
            "\t-p flavor:  perft flavor, 1:perft, 2:perft_alt 3:both, default:1\n");
    return 1;
}

int main(int ac, char**av)
{
    int curtest = 0, totalfen = 0;
    u64 sf_count = 0, my_count;
    bool comment = true, sf_run = false, divide = false;
    char *fen;
    pos_t *pos = NULL, *fenpos;
    pos_t *fishpos = pos_new();
    movelist_t fishmoves;
    FILE *outfd = NULL;
    s64 ms, lps;
    int opt, depth = 6, run = 3, tt, newtt = HASH_SIZE_DEFAULT;
    int startline = 1, ntests = INT_MAX;

    struct {
        s64 count, countskipped, ms;
        s64 minlps, maxlps;
        int skipped;
        int err;
    } res[3] = {
        { .minlps = LONG_MAX },
        { .minlps = LONG_MAX },
        { .minlps = LONG_MAX },
    };

    printf("Perft " VERSION "\n");
    while ((opt = getopt(ac, av, "cd:l:mn:p:st:")) != -1) {
        switch (opt) {
            case 'c':
                comment = false;
                break;
            case 'd':
                depth = atoi(optarg);
                if (depth <= 0)
                    depth = 6;
                break;
            case 'l':
                startline = atoi(optarg);
                break;
            case 'm':
                divide = true;
                break;
            case 'n':
                ntests = atoi(optarg);
                break;
            case 'p':
                run = atoi(optarg);
                break;
            case 's':
                sf_run = true;
                break;
            case 't':
                newtt = atoi(optarg);
                break;
            default:
                    return usage(*av);
        }
    }

    if (!run) {
        printf("Nothing to do, exiting\n");
        exit(0);
    }

    init_all();
    tt = hash_tt.mb;

    if (run & 1 && newtt != tt) {
        tt_create(newtt);

        printf("changing TT size from %d to %d\n", tt, newtt);
        tt = newtt;
    }
    printf("%s: depth:%d tt_size:%d run:%x SF:%s\n",
           *av,
           depth, newtt, run,
           sf_run? "yes": "no");

    tt_info();
    printf("\n");

    if (sf_run)
        outfd = open_stockfish();

    /* count total fen tests we will do */
    while (next_fen(PERFT | MOVEDO))
        totalfen++;
    restart_fen();

    CLOCK_DEFINE(clock, CLOCK_MONOTONIC);
    while ((fen = next_fen(PERFT | MOVEDO))) {
        if (cur_line() < startline)
            continue;
        if (curtest >= ntests)
            break;
        if (!(fenpos = fen2pos(pos, fen))) {
            printf("wrong fen line:%d fen:%s\n\n", cur_line(), fen);
            continue;
        }
        curtest++;
        printf("test:%d/%d line:%d fen:%s\n", curtest, totalfen, cur_line(), fen);
        if (comment)
            printf("\t\"%s\"\n",
                   *cur_comment()? cur_comment(): "no test desc");

        tt_clear();

        pos = fenpos;
        if (sf_run) {
            stockfish_fen(outfd, fen);
            clock_start(&clock);
            sf_count = stockfish_perft(outfd, fishpos, &fishmoves, depth, divide);
            ms = clock_elapsed_ms(&clock);
            if (!ms) {
                res[2].skipped++;
                res[2].countskipped += sf_count;
                lps = 0;
            } else {
                lps = sf_count * 1000l / ms;
                res[2].ms += ms;
                res[2].count += sf_count;
                if (lps > res[2].maxlps)
                    res[2].maxlps = lps;
                if (lps < res[2].minlps)
                    res[2].minlps = lps;
            }
            printf("Stockfish : perft:%'lu ms:%'ld lps:%'lu\n",
                   sf_count, ms, lps);
        }

        if (run & 1) {
            clock_start(&clock);
            my_count = perft(pos, depth, 1, divide);
            ms = clock_elapsed_ms(&clock);
            if (!ms) {
                res[0].skipped++;
                res[0].countskipped += my_count;
                lps = 0;
            } else {
                lps = my_count * 1000l / ms;
                res[0].ms += ms;
                res[0].count += my_count;
                if (lps > res[0].maxlps)
                    res[0].maxlps = lps;
                if (lps < res[0].minlps)
                    res[0].minlps = lps;
            }

            if (!sf_run || sf_count == my_count) {
                printf("perft     : perft:%'lu ms:%'ld lps:%'lu ",
                       my_count, ms, lps);
                tt_stats();
            } else  {
                printf("perft     : perft:%'lu ***ERROR***\n", my_count);
                res[0].err++;
            }
        }

        if (run & 2) {
            clock_start(&clock);
            my_count = perft_alt(pos, depth, 1, divide);
            ms = clock_elapsed_ms(&clock);
            if (!ms) {
                res[1].skipped++;
                res[1].countskipped += my_count;
                lps = 0;
            } else {
                lps = my_count * 1000l / ms;
                res[1].ms += ms;
                res[1].count += my_count;
                if (lps > res[1].maxlps)
                    res[1].maxlps = lps;
                if (lps < res[1].minlps)
                    res[1].minlps = lps;
            }

            if (!sf_run || sf_count == my_count) {
                printf("perft_alt : perft:%'lu ms:%'ld lps:%'lu\n",
                       my_count, ms, lps);
            } else  {
                printf("perft_alt : perft:%'lu ***ERROR***\n", my_count);
                res[1].err++;
            }
        }
        printf("\n");
    }
    pos_del(pos);
    if (sf_run) {
        if (!res[2].ms)
            res[2].ms = 1;
        printf("total Stockfish : perft:%'lu ms:%'lu lps:%'lu min:%'lu max:%'lu "
               "(skipped %d/%d)\n",
               res[2].count + res[2].countskipped, res[2].ms,
               res[2].count * 1000l / res[2].ms,
               res[2].minlps, res[2].maxlps,
               res[2].skipped, curtest);
    }
    if (run & 1) {
        if (!res[0].ms)
            res[0].ms = 1;
        printf("total perft     : perft:%'lu ms:%'lu lps:%'lu min:%'lu max:%'lu "
               "(pos:%d skipped:%d err:%d)\n",
               res[0].count + res[0].countskipped, res[0].ms,
               res[0].count * 1000l / res[0].ms,
               res[0].minlps, res[0].maxlps,
               curtest, res[0].skipped, res[0].err);
    }
    if (run & 2) {
        if (!res[1].ms)
            res[1].ms = 1;
        printf("total perft_alt : perft:%'lu ms:%'lu lps:%'lu min:%'lu max:%'lu "
               "(pos:%d skipped:%d err:%d)\n",
               res[1].count + res[1].countskipped, res[1].ms,
               res[1].count * 1000l / res[1].ms,
               res[1].minlps, res[1].maxlps,
               curtest, res[1].skipped, res[1].err);
    }
    return 0;
}
