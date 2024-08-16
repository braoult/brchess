/* uci.c - uci protocol
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
#include <unistd.h>
#include <ctype.h>

#include <brlib.h>
#include <bug.h>

#include "chessdefs.h"
#include "util.h"
#include "position.h"
#include "hist.h"
#include "fen.h"
#include "move-gen.h"
#include "move-do.h"
#include "search.h"
#include "perft.h"
#include "eval-defs.h"
#include "uci.h"

struct command {
    char *name;                                   /* command name */
    int (*func)(pos_t *, char *);                 /* function doing the job */
    char *doc;                                    /* function doc */
};

int execute_line (pos_t *, struct command *, char *);
struct command *find_command (char *);

/* The names of functions that actually do the stuff.
 */

/* standard UCI commands */
int do_ucinewgame(pos_t *, char *);
int do_uci(pos_t *, char *);
int do_isready(pos_t *, char *);
int do_quit(pos_t *, char *);

int do_setoption(pos_t *, char *);
int do_position(pos_t *, char *);
int do_go(pos_t *, char *);

/* commands *NOT* in UCI standard */
int do_moves(pos_t *, char *);
int do_diagram(pos_t *, char *);
int do_perft(pos_t *, char *);

int do_hist(pos_t *, char *);
int do_help(pos_t *, char *);

struct command commands[] = {
    { "quit",       do_quit, "Quit" },
    { "uci",        do_uci, "" },
    { "ucinewgame", do_ucinewgame, "" },
    { "isready",    do_isready, "" },
    { "setoption",  do_setoption, ""},
    { "position",   do_position, "position startpos|fen [moves ...]" },
    { "go",         do_go, "go" },


    { "perft",      do_perft, "(not UCI) perft [divide] [alt] depth" },
    { "moves",      do_moves, "(not UCI) moves ..." },
    { "diagram",    do_diagram, "(not UCI) print current position diagram" },
    { "hist",       do_hist, "(not UCI) print history states" },
    { "help",       do_help, "(not UCI) This help" },
    { "?",          do_help, "(not UCI) This help" },

    { NULL, (int(*)()) NULL, NULL }
};

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
 * command.  Return a NULL pointer if NAME isn't a command name.
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
    return 1;
}

int do_uci(__unused pos_t *pos, __unused char *arg)
{
    printf("id name brchess " VERSION "\n");
    printf("id author Bruno Raoult\n");
    printf("option name Hash type spin default %d min %d max %d\n",
           hash_tt.mb, HASH_SIZE_MIN, HASH_SIZE_MAX);

    if (PST_NB > 1) {
        printf("option name pst type combo default %s",
               pst_name(pst_current));
        for (int i = 0; i < PST_NB; ++i)
            printf(" var %s", pst_name(i));
    }
    printf("\n");
    for (int var = 0; var < EV_PARAMS_NB; ++var) {
        if (param_setable(var)) {
            int ptyp = param_type(var);
            int pmin = param_min(var);
            int pmax = param_max(var);
            int pval = parameters[var];

            printf("option name %s ", param_name(var));
            switch(ptyp) {
                case PAR_BTN:
                    printf("type button\n");
                    break;
                case PAR_CHK:
                    printf("type check default %s\n", pval? "true": "false");
                    break;
                case PAR_SPN:
                    printf("type spin default %d min %d max %d\n", pval, pmin, pmax);
                    break;
            };
        }
    }
    printf("uciok\n");
    return 1;
}

int do_isready(__unused pos_t *pos, __unused char *arg)
{
    printf("readyok\n");
    return 1;
}

int do_setoption(__unused pos_t *pos, __unused char *arg)
{
    char *name, *value = NULL;

    if (str_token(arg, "name") != arg)
        return 1;
    if (!(name = str_skip_word(arg)))
        return 1;
    /* at this point, we got a valid parameter name */
    value = str_token(name, "value");             /* put '\0' at name end */
    if (value) {
        value = str_skip_word(value);
        if (!value)
            return 1;
    }
    if (str_eq_case(name, "hash") && value) {
        tt_create(atoi(value));
    } else if (str_eq_case(name, "pst")) {
        pst_set(value);
    } else {
        int var = param_find_name(name);
        if (var < 0) {
            printf("wrong parameter '%s'\n", name);
            return 1;
        }
        char *pname = param_name(var);
        printf("found param <%s> = %d\n", pname, var);
        if (param_setable(var)) {
            int ptyp = param_min(var);
            int pmin = param_min(var);
            int pmax = param_max(var);
            int pval;

            switch(ptyp) {
                case PAR_BTN:
                    bug_on (value);
                    printf("do button '%s'\n", pname);
                    break;
                case PAR_CHK:
                    bug_on (!value);
                    if (str_eq_case(value, "true"))
                        pval = 1;
                    else if (str_eq_case(value, "false"))
                        pval = 0;
                    else {
                        printf("wrong value '%s' to '%s' boolean parameter\n",
                               value, pname);
                        return 1;
                    }
                    printf("set '%s' to %s\n", pname, pval? "true": "false");
                    param_set(var, pval);
                    break;
                case PAR_SPN:
                    bug_on (!value);
                    pval = clamp(atoi(value), pmin, pmax);
                    printf("set '%s' to %d\n", param_name(var), pval);
                    param_set(var, pval);
                    break;
            }
        }
    }

    return 0;
}


int do_position(pos_t *pos, char *arg)
{
    char *saveptr = NULL, *token, *fen, *moves;

    hist_init();

    /* separate "moves" section */
    moves = str_token(arg, "moves");
    token = strtok_r(arg, " ", &saveptr);
    if (!strcmp(token, "startpos")) {
        startpos(pos);
        do_diagram(pos, "");
    } else if (!strcmp(token, "fen")) {
        fen = strtok_r(NULL, "", &saveptr);       /* full fen (till '\0') */
        //printf("fen=%s\n", fen);
        if (!fen)
            return 1;
        if (!fen2pos(pos, fen))
            return 1;
        //do_diagram(pos, "");
    } else {
        return 1;
    }
    //puts("zob");
    //move_t move_none = MOVE_NONE;
    //hist_push(&pos->state, &move_none);

    if (moves) {
        //puts("zobi");
        saveptr = NULL;
        moves = strtok_r(moves, " ", &saveptr);   /* skip "moves" */
        moves = strtok_r(NULL, "", &saveptr);     /* all moves (till '\0') */
        //printf("moves = %s\n", moves);
        do_moves(pos, moves);
    }
    /* link last position t history */
    //hist_pop();
    //hist_link(pos);
    return 1;
}


int do_go(pos_t *pos, char *arg)
{
    char *ptr = NULL, *tok, *val;

    //token = strtok_r(arg, " ", &saveptr);
    for (tok = strtok_r(arg, " ", &ptr); tok; tok = strtok_r(arg, " ", &ptr)) {
        /* TODO: Find a "clever" way to get the different values without
         * multiple "strcmp"
         */

        if (!strcmp(tok, "searchmoves")) {        /* moves list */
            ;
        } else if (!strcmp(tok, "wtime")) {       /* integer */
            ;
        } else if (!strcmp(tok, "btime")) {       /* integer */
            ;
        } else if (!strcmp(tok, "winc")) {        /* integer */
            ;
        } else if (!strcmp(tok, "binc")) {        /* integer */
            ;
        } else if (!strcmp(tok, "movestogo")) {   /* integer */
            ;
        } else if (!strcmp(tok, "depth")) {       /* integer */
            if ((val = strtok_r(arg, " ", &ptr)))
                search_uci.depth = atoi(val);
            ;
        } else if (!strcmp(tok, "nodes")) {       /* integer */
            ;
        } else if (!strcmp(tok, "mate")) {        /* integer */
            ;
        } else if (!strcmp(tok, "movetime")) {    /* integer */
            ;
        } else if (!strcmp(tok, "ponder")) {      /* no param */
            ;
        } else if (!strcmp(tok, "infinite")) {    /* no param */
            ;
        }

    }
    search(pos);
    return 1;
}

int do_moves(__unused pos_t *pos, char *arg)
{
    char *saveptr = NULL, *token, check[8];
    move_t move;
    movelist_t movelist;

    token = strtok_r(arg, " ", &saveptr);
    while (token) {
        move = move_from_str(token);
        move_to_str(check, move, 0);

        printf("move: [%s] %s\n", token, check);
        pos_set_checkers_pinners_blockers(pos);
        pos_legal(pos, pos_gen_pseudo(pos, &movelist));
        move = move_find_in_movelist(move, &movelist);
        if (move == MOVE_NONE) {
            /* should we reset here ? */
            return 1;
        }
        //printf("move: %s\n", move_to_str(check, move, 0));
        //hist_push(&pos->state);                   /* push previous state */
        move_do(pos, move, hist_next());
        printf("repet=%d\n", pos->repcount);
        //if (is_repetition(pos))
        //    printf("rep detected\n");
        //else if(is_draw(pos))
        //    printf("draw detected\n");
        hist_static_print();
        token = strtok_r(NULL, " ", &saveptr);
    }
    /* reset position root, and decrease history moves repcounts.
     * TODO: Maybe use "ucinewgame" to decide when to perform this decrease ?
     */
    pos->plyroot = 0;
    for (state_t *st = &pos->state; st != HIST_START; st = hist_prev(st)) {
        //printf("adjust rep=%d->\n");
        st->repcount = max(0, st->repcount - 1);
    }
    pos_print(pos);
    hist_print(pos);
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
            perft(pos, depth, divide);
        else
            perft_alt(pos, depth, 1, divide);
    }
    return 1;
}

int do_hist(__unused pos_t *pos, __unused char *arg)
{
    hist_print(pos);
    return 0;
}

int do_help(__unused pos_t *pos, __unused char *arg)
{
    for (struct command *cmd = commands; cmd->name; ++cmd) {
        printf("%12s:\t%s\n", cmd->name, cmd->doc);
        /* Print in six columns. */
    }

    return 0;
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


static int done = 0;

int do_quit(__unused pos_t *pos, __unused char *arg)
{
    return done = 1;
}

int uci(pos_t *pos)
{
    char *str = NULL, *saveptr, *token, *args;
    size_t lenstr = 0;
    struct command *command;

    while (!done && getline(&str, &lenstr, stdin) >= 0) {
        str = str_trim(str);
        if (! *str)
            continue;
        token = strtok_r(str, " ", &saveptr);
        if (! (command = find_command(token))) {
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
