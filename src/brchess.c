/* brchess.c - main loop.
 *
 * Copyright (C) 2021-2024 Bruno Raoult ("br")
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
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include <brlib.h>

#include "chessdefs.h"
#include "position.h"
#include "brchess.h"
#include "hash.h"
#include "fen.h"
#include "search.h"
#include "hist.h"
#include "move-gen.h"
#include "move-do.h"

struct command {
    char *name;                                   /* User printable name */
    int (*func)(pos_t *, char *);                 /* function doing the job */
    char *doc;                                    /* function doc */
};

int execute_line (pos_t *, struct command *, char *);
struct command *find_command (char *);
int string_trim (char *str);

/* The names of functions that actually do the manipulation. */
int do_ucinewgame(pos_t *, char *);
int do_uci(pos_t *, char *);
int do_isready(pos_t *, char *);

int do_position(pos_t *, char *);
int do_moves(pos_t *, char *);
int do_diagram(pos_t *, char *);
int do_perft(pos_t *, char *);

int do_help(pos_t *, char *);
int do_quit(pos_t *, char *);

struct command commands[] = {
    { "help",       do_help, "(not UCI) This help" },
    { "?",          do_help, "(not UCI) This help" },
    { "quit",       do_quit, "Quit" },

    { "uci",        do_uci, "" },
    { "ucinewgame", do_ucinewgame, "" },
    { "isready",    do_isready, "" },

    { "position",   do_position, "position startpos|fen [moves ...]" },

    { "perft",      do_perft, "(not UCI) perft [divide] [alt] depth" },
    { "moves",      do_moves, "(not UCI) moves ..." },
    { "diagram",    do_diagram, "(not UCI) print current position diagram" },

 /*
 *     { "init", do_init, "Set position to normal start position" },

 *     { "genmove", do_genmoves, "Generate move list for " },
 *     { "prmoves", do_prmoves, "Print position move list" },
 * //    { "prmovepos", do_prmovepos, "Print Nth move resulting position" },
 *     { "prpieces", do_prpieces, "Print Pieces (from pieces lists)" },
 *     { "memstats", do_memstats, "Generate next move list" },
 *     { "eval", do_eval, "Eval current position" },
 *     { "simple-eval", do_simple_eval, "Simple eval current position" },
 *     { "do_move", do_move, "execute nth move on current position" },
 *     { "debug", do_debug, "Set log level to LEVEL" },
 *     { "depth", do_depth, "Set search depth to N" },
 *     { "search", do_search, "Search best move (negamax)" },
 *     { "pvs", do_pvs, "Search best move (Principal Variation Search)" },
 */
    { NULL, (int(*)()) NULL, NULL }
};

static int done = 0;

int brchess(pos_t *pos)
{
    char *str = NULL, *saveptr, *token, *args;
    int len;
    size_t lenstr = 0;
    struct command *command;

    while (!done && getline(&str, &lenstr, stdin) >= 0) {
        if (!(len = string_trim(str)))
            continue;
        token = strtok_r(str, " ", &saveptr);
        if (! (command= find_command(token))) {
            fprintf(stderr, "Unknown [%s] command. Try 'help'.\n", token);
            continue;
        }
        args = strtok_r(NULL, "", &saveptr);
        execute_line(pos, command, args);
    }

    if (str)
        free(str);

    return 0;
}

/* Execute a command line. */
int execute_line(pos_t *pos, struct command *command, char *args)
{
    return (*command->func)(pos, args);
}

/**
 * find_command - lookup UCI command.
 * @name: &command string
 *
 * Look up NAME as the name of a command, and return a pointer to that
 *   command.  Return a NULL pointer if NAME isn't a command name.
 */
struct command *find_command(char *name)
{
    register int i;

    for (i = 0; commands[i].name; i++)
        if (!strcmp(name, commands[i].name))
            return commands + i;

    return NULL;
}

/*
 * int do_eval(__unused pos_t *pos, __unused char *arg)
 * {
 *     eval_t material[2], control[2], mobility[2];
 *     for (int color = WHITE; color <= BLACK; ++color) {
 *         material[color] = eval_material(pos, color);
 *         control[color] = eval_square_control(pos, color);
 *         mobility[color] = eval_mobility(pos, color);
 *         printf("%s: material=%d mobility=%d controlled=%d\n",
 *                color? "Black": "White", material[color],
 *                mobility[color], control[color]);
 *     }
 *     eval_t res = eval(pos);
 *     printf("eval = %d centipawns\n", res);
 *     return 1;
 * }
 *
 * int do_simple_eval(__unused pos_t *pos, __unused char *arg)
 * {
 *     eval_t eval = eval_simple(pos);
 *     printf("eval = %d centipawns\n", eval);
 *     return 1;
 * }
 */

/*
 * int do_init(pos_t *pos, __unused char *arg)
 * {
 *     startpos(pos);
 *     return 1;
 * }
 */

/*
 * int do_genmoves(pos_t *pos, __unused char *arg)
 * {
 *     moves_gen_all(pos);
 *     return 1;
 * }
 *
 * int do_prmoves(pos_t *pos, __unused char *arg)
 * {
 *     uint debug_level = debug_level_get();
 *     debug_level_set(1);
 *     moves_print(pos, M_PR_SEPARATE | M_PR_NUM | M_PR_LONG);
 *     debug_level_set(debug_level);
 *     return 1;
 * }
 */

/*
 * int do_prmovepos(pos_t *pos, char *arg)
 * {
 *     struct list_head *p_cur, *tmp;
 *     int movenum = atoi(arg), cur = 0;             /\* starts with 0 *\/
 *     move_t *move;
 *
 *     log_f(1, "%s\n", arg);
 *     list_for_each_safe(p_cur, tmp, &pos->moves[pos->turn]) {
 *         move = list_entry(p_cur, move_t, list);
 *         if (cur++ == movenum) {
 *             pos_print(move->newpos);
 *             break;
 *         }
 *     }
 *
 *     return 1;
 * }
 */

/*
 * int do_prpieces(pos_t *pos, __unused char *arg)
 * {
 *     log_f(1, "%s\n", arg);
 *     pos_pieces_print(pos);
 *     return 1;
 * }
 *
 * int do_memstats(__unused pos_t *pos,__unused char *arg)
 * {
 *     moves_pool_stats();
 *     piece_pool_stats();
 *     pos_pool_stats();
 *     return 1;
 * }
 */

/*
 * int do_move(__unused pos_t *pos, __unused char *arg)
 * {
 *     int i = 1, nmove = atoi(arg);
 *     move_t *move;
 *     pos_t *newpos;
 *
 *     if (list_empty(&pos->moves[pos->turn])) {
 *         log_f(1, "No moves list.\n");
 *         return 0;
 *     }
 *     list_for_each_entry(move, &pos->moves[pos->turn], list) {
 *         if (i == nmove)
 *             goto doit;
 *         i++;
 *     }
 *     log_f(1, "Invalid <%d> move, should be <1-%d>.\n", nmove, i);
 *     return 0;
 * doit:
 *     newpos = move_do(pos, move);
 *     pos_print(newpos);
 *
 *     return 1;
 * }
 */

int do_ucinewgame(__unused pos_t *pos, __unused char *arg)
{
    pos_clear(pos);
    tt_clear();
    hist_init();
    return 1;
}

int do_uci(__unused pos_t *pos, __unused char *arg)
{
    printf("id name brchess " VERSION "\n");
    printf("id author Bruno Raoult\n");
    printf("option option name Hash type spin default %d min %d max %d\n",
           hash_tt.mb, HASH_SIZE_MIN, HASH_SIZE_MAX);
    printf("uciok\n");
    return 1;
}

int do_isready(__unused pos_t *pos, __unused char *arg)
{
    printf("readyok\n");
    return 1;
}

int do_position(pos_t *pos, char *arg)
{
    char *saveptr, *token, *fen, *moves;

    hist_init();

    /* separate "moves" section */
    if ((moves = strstr(arg, "moves"))) {
        *(moves - 1) = 0;
    }
    saveptr = NULL;
    token = strtok_r(arg, " ", &saveptr);
    if (!strcmp(token, "startpos")) {
        startpos(pos);
        do_diagram(pos, "");
    } else if (!strcmp(token, "fen")) {
        fen = strtok_r(NULL, " ", &saveptr);
        fen2pos(pos, fen);
    } else {
        puts("fuck");
    }

    if (moves) {
        saveptr = NULL;
        moves = strtok_r(moves, " ", &saveptr);
        moves = strtok_r(NULL, "", &saveptr);
        printf("moves = %s\n", moves);
        do_moves(pos, moves);
    }

    return 1;
}

static move_t *move_find_move(move_t target, movelist_t *list)
{
    move_t *move = list->move, *last = move + list->nmoves;

    for (; move < last; ++move) {
        if (move_from(target) == move_from(*move) &&
            move_to(target)   == move_to(*move) &&
            move_to(target)   == move_to(*move) &&
            move_promoted(target)   == move_promoted(*move))
            return move;
    }
    return NULL;
}

int do_moves(__unused pos_t *pos, char *arg)
{
    char *saveptr = NULL, *token, check[8];
    move_t move, *foundmove;
    state_t state;
    movelist_t movelist;
    saveptr = NULL;
    token = strtok_r(arg, " ", &saveptr);
    while (token) {
        move = move_from_str(pos, token);
        move_to_str(check, move, 0);

        printf("move: [%s] %s\n", token, check);
        pos_set_checkers_pinners_blockers(pos);
        pos_legal(pos, pos_gen_pseudo(pos, &movelist));
        foundmove = move_find_move(move, &movelist);
        if (!foundmove) {
            printf("illegal move");
            return 1;
        }
        move_do(pos, *foundmove, &state);
        hist_push(&state, &move);
        token = strtok_r(NULL, " ", &saveptr);
    }
    hist_static_print();
    return 1;
}

int do_diagram(pos_t *pos, __unused char *arg)
{
    pos_print(pos);
    return 1;
}

int do_perft(__unused pos_t *pos, __unused char *arg)
{
    char *saveptr, *token;
    int divide = 0, depth = 6, alt = 0;

    token = strtok_r(arg, " ", &saveptr);
    if (!strcmp(token, "divide")) {
        divide = 1;
        token = strtok_r(NULL, " ", &saveptr);
    }
    if (!strcmp(token, "alt")) {
        alt = 1;
        token = strtok_r(NULL, " ", &saveptr);
    }
    depth = atoi(token);
    printf("perft: divide=%d alt=%d depth=%d\n", divide, alt, depth);
    if (depth > 0) {
        if (!alt)
            perft(pos, depth, 1, divide);
        else
            perft_alt(pos, depth, 1, divide);
    }
    return 1;
}

/*
 * int do_debug(__unused pos_t *pos, __unused char *arg)
 * {
 *     debug_level_set(atoi(arg));
 *     return 1;
 * }
 */

int do_help(__unused pos_t *pos, __unused char *arg)
{
    for (struct command *cmd = commands; cmd->name; ++cmd) {
        printf("%12s:\t%s\n", cmd->name, cmd->doc);
        /* Print in six columns. */
    }

    return 0;
}

int do_quit(__unused pos_t *pos, __unused char *arg)
{
    return done = 1;
}

/*
 * int do_depth(__unused pos_t *pos, char *arg)
 * {
 *     depth = atoi(arg);
 *     printf("depth = %d\n", depth);
 *     return 1;
 *
 * }
 *
 * int do_search(pos_t *pos, __unused char *arg)
 * {
 *     int debug_level = debug_level_get();
 *     float timer1, timer2, nodes_sec;
 *
 *     timer1 = debug_timer_elapsed();
 *     negamax(pos, depth, pos->turn == WHITE ? 1 : -1);
 *     timer2 = debug_timer_elapsed();
 *     nodes_sec = (float) pos->node_count / ((float) (timer2 - timer1) / (float)NANOSEC);
 *     log(1, "best=");
 *     debug_level_set(1);
 *     move_print(0, pos->bestmove, 0);
 *     debug_level_set(debug_level);
 *     log(1, " negamax=%d\n", pos->bestmove->negamax);
 *     printf("Depth:%d Nodes:%luK time:%.02fs (%.0f kn/s)\n", depth,
 *            pos->node_count / 1000, (timer2 - timer1)/NANOSEC, nodes_sec/1000);
 *     return 1;
 * }
 */

/*
 * int do_pvs(pos_t *pos, __unused char *arg)
 * {
 *     int debug_level = debug_level_get();
 *     float timer1, timer2, nodes_sec;
 *     eval_t _pvs;
 *
 *     timer1 = debug_timer_elapsed();
 *     moves_gen_eval_sort(pos);
 *     _pvs = pvs(pos, depth, EVAL_MIN, EVAL_MAX, pos->turn == WHITE ? 1 : -1);
 *     timer2 = debug_timer_elapsed();
 *     nodes_sec = (float) pos->node_count / ((float) (timer2 - timer1) / (float)NANOSEC);
 *     log(1, "best=");
 *     if (pos->bestmove) {
 *         debug_level_set(1);
 *         move_print(0, pos->bestmove, 0);
 *         debug_level_set(debug_level);
 *         log(1, " pvs=%d stored=%d\n", _pvs, pos->bestmove->negamax);
 *     } else {
 *         log(1, "<no-best-move>");
 *     }
 *     printf("Depth:%d Nodes:%luK time:%.02fs (%.0f kn/s)\n", depth,
 *            pos->node_count / 1000, (timer2 - timer1)/NANOSEC, nodes_sec/1000);
 *     return 1;
 * }
 */

/**
 * string_trim - cleanup (trim) blank characters in string.
 * @str: &string to clean
 *
 * str is cleaned and packed with the following rules:
 * - Leading and trailing blank characters are removed.
 * - consecutive blank characters are replaced by one space.
 * - non printable characters are removed.
 *
 * "blank" means characters as understood by isspace(3): space, form-feed ('\f'),
 * newline ('\n'), carriage return ('\r'), horizontal tab  ('\t'), and vertical
 * tab ('\v').
 *
 * @return: new @str len.
 */
int string_trim(char *str)
{
    char *to = str, *from = str;
    int state = 1;

    while (*from) {
        switch (state) {
            case 1:                               /* blanks */
                while (*from && isspace(*from))
                    from++;
                state = 0;
                break;
            case 0:                               /* token */
                while (*from && !isspace(*from)) {
                    if (isprint(*from))
                        *to++ = *from;
                    from++;
                }
                *to++ = ' ';
                state = 1;
        }
    }
    if (to > str)
        to--;
    *to = 0;
    return to - str;
}


/**
 * usage - brchess usage function.
 *
 */
static int usage(char *prg)
{
    fprintf(stderr, "Usage: %s [-ilw] [file...]\n", prg);
    return 1;
}

int main(int ac, char **av)
{
    pos_t *pos = pos_new();
    int opt;

    printf("brchess " VERSION "\n");
    init_all();

    // size_t len = 0;
    // char *str = NULL;
    //while (getline(&str, &len, stdin) >= 0) {
    //    printf("[%s] -> ", str);
    //    int newlen = string_trim(str);
    //    printf("%d [%s]\n", newlen, str);
    //}
    //exit(0);
    while ((opt = getopt(ac, av, "d:f:")) != -1) {
        switch (opt) {
            case 'd':
                //debug_level_set(atoi(optarg));
                break;
            case 'f':
                fen2pos(pos, optarg);
                break;
            default:
                return usage(*av);
        }
    }
    if (optind < ac)
        return usage(*av);

    return brchess(pos);
}
