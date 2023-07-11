/* brchess.c - main loop.
 *
 * Copyright (C) 2021 Bruno Raoult ("br")
 * Licensed under the GNU General Public License v3.0 or later.
 * Some rights reserved. See COPYING.
 *
 * You should have received a copy of the GNU General Public License along with this
 * program. If not, see <https://www.gnu.org/licenses/gpl-3.0-standalone.html>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later <https://spdx.org/licenses/GPL-3.0-or-later.html>
 *
 */


#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#include <br.h>
#include <list.h>
#include <debug.h>

#include "brchess.h"
#include "chessdefs.h"
#include "board.h"
#include "piece.h"
#include "move.h"
#include "fen.h"
#include "eval.h"
#include "eval-simple.h"
#include "search.h"

struct command {
    char *name;                                   /* User printable name */
    int (*func)(pos_t *, char *);                 /* function doing the job */
    char *doc;                                    /* function doc */
};

/* readline example inspired by :
 * - https://thoughtbot.com/blog/tab-completion-in-gnu-readline
 * - http://web.mit.edu/gnu/doc/html/rlman_2.html
 */
char **commands_completion(const char *, int, int);
char *commands_generator(const char *, int);
char *escape(const char *);
int quote_detector(char *, int);
int execute_line (pos_t *, char *line);
struct command *find_command (char *);
char *stripwhite (char *string);

/* The names of functions that actually do the manipulation. */
int do_help(pos_t *, char*);
int do_fen(pos_t *, char*);
int do_init(pos_t *, char*);
int do_pos(pos_t *, char*);
int do_genmoves(pos_t *, char*);
int do_prmoves(pos_t *, char*);
//int do_prmovepos(pos_t *pos, char *arg);
int do_prpieces(pos_t *pos, char *arg);
int do_memstats(pos_t *, char*);
int do_eval(pos_t *, char*);
int do_simple_eval(pos_t *, char*);
int do_move(pos_t *, char*);
int do_quit(pos_t *, char*);
int do_debug(pos_t *, char*);
int do_depth(pos_t *, char*);
int do_search(pos_t *, char*);
int do_pvs(pos_t *, char*);

struct command commands[] = {
    { "help", do_help, "Display this text" },
    { "?", do_help, "Synonym for 'help'" },
    { "fen", do_fen, "Set position to FEN" },
    { "init", do_init, "Set position to normal start position" },
    { "pos", do_pos, "Print current position" },
    { "quit", do_quit, "Quit" },
    { "genmove", do_genmoves, "Generate move list for " },
    { "prmoves", do_prmoves, "Print position move list" },
//    { "prmovepos", do_prmovepos, "Print Nth move resulting position" },
    { "prpieces", do_prpieces, "Print Pieces (from pieces lists)" },
    { "memstats", do_memstats, "Generate next move list" },
    { "eval", do_eval, "Eval current position" },
    { "simple-eval", do_simple_eval, "Simple eval current position" },
    { "do_move", do_move, "execute nth move on current position" },
    { "debug", do_debug, "Set log level to LEVEL" },
    { "depth", do_depth, "Set search depth to N" },
    { "search", do_search, "Search best move (negamax)" },
    { "pvs", do_pvs, "Search best move (Principal Variation Search)" },
    { NULL, (int(*)()) NULL, NULL }
};

static int done=0;
static int depth=1;

int brchess(pos_t *pos)
{
    char *buffer, *s;

    rl_attempted_completion_function = commands_completion;
    rl_completer_quote_characters = "'\"";
    rl_completer_word_break_characters = " ";
    rl_char_is_quoted_p = &quote_detector;

    while (!done) {
        buffer = readline("chess> ");
        if (!buffer)
            break;
        /* Remove leading and trailing whitespace from the line.
         * Then, if there is anything left, add it to the history list
         * and execute it.
         */
        s = stripwhite(buffer);

        if (*s) {
            add_history(s);
            execute_line(pos, s);
        }
        free(buffer);
    }

    return 0;
}

//char **commands_completion(const char *text, int start, int end)
char **commands_completion(const char *text, __unused int start, __unused int end)
{
    rl_attempted_completion_over = 1;
    return rl_completion_matches(text, commands_generator);
}

char *commands_generator(const char *text, int state)
{
    static int list_index, len;
    char *name;

    if (!state) {
        list_index = 0;
        len = strlen(text);
    }

    while ((name = commands[list_index++].name)) {
        if (rl_completion_quote_character) {
            name = strdup(name);
        } else {
            name = escape(name);
        }

        if (strncmp(name, text, len) == 0) {
            return name;
        } else {
            free(name);
        }
    }

    return NULL;
}

char *escape(const char *original)
{
    size_t original_len;
    size_t i, j;
    char *escaped, *resized_escaped;

    original_len = strlen(original);

    if (original_len > SIZE_MAX / 2) {
        errx(1, "string too long to escape");
    }

    if ((escaped = malloc(2 * original_len + 1)) == NULL) {
        err(1, NULL);
    }

    for (i = 0, j = 0; i < original_len; ++i, ++j) {
        if (original[i] == ' ') {
            escaped[j++] = '\\';
        }
        escaped[j] = original[i];
    }
    escaped[j] = '\0';

    if ((resized_escaped = realloc(escaped, j)) == NULL) {
        free(escaped);
        resized_escaped = NULL;
        err(1, NULL);
    }

    return resized_escaped;
}

int quote_detector(char *line, int index)
{
    return index > 0
            && line[index - 1] == '\\'
            &&!quote_detector(line, index - 1);
}

/* Execute a command line. */
int execute_line(pos_t *pos, char *line)
{
    register int i;
    struct command *command;
    char *word;

    /* Isolate the command word. */
    i = 0;
    while (line[i] && whitespace(line[i]))
        i++;
    word = line + i;

    while (line[i] && !whitespace(line[i]))
        i++;

    if (line[i])
        line[i++] = '\0';

    command = find_command(word);

    if (!command) {
        fprintf(stderr, "%s: Unknown command.\n", word);
        return -1;
    }

    /* Get argument to command, if any. */
    while (whitespace(line[i]))
        i++;

    word = line + i;

    /* return command number */
    return (*command->func)(pos, word);
}

/* Look up NAME as the name of a command, and return a pointer to that
   command.  Return a NULL pointer if NAME isn't a command name. */
struct command *find_command(char *name)
{
    register int i;

    for (i = 0; commands[i].name; i++)
        if (strcmp(name, commands[i].name) == 0)
            return &commands[i];

    return (struct command *)NULL;
}

/* Strip whitespace from the start and end of STRING.  Return a pointer
   into STRING. */
char *stripwhite(char *string)
{
    register char *s, *t;

    for (s = string; whitespace(*s); s++)
        ;

    if (*s == 0)
        return s;

    t = s + strlen(s) - 1;
    while (t > s && whitespace(*t))
        t--;
    *++t = '\0';

    return s;
}

int do_eval(__unused pos_t *pos, __unused char *arg)
{
    eval_t material[2], control[2], mobility[2];
    for (int color = WHITE; color <= BLACK; ++color) {
        material[color] = eval_material(pos, color);
        control[color] = eval_square_control(pos, color);
        mobility[color] = eval_mobility(pos, color);
        printf("%s: material=%d mobility=%d controlled=%d\n",
               color? "Black": "White", material[color],
               mobility[color], control[color]);
    }
    eval_t res = eval(pos);
    printf("eval = %d centipawns\n", res);
    return 1;
}

int do_simple_eval(__unused pos_t *pos, __unused char *arg)
{
    eval_t eval = eval_simple(pos);
    printf("eval = %d centipawns\n", eval);
    return 1;
}

int do_fen(pos_t *pos, char *arg)
{
    fen2pos(pos, arg);
    return 1;
}

int do_init(pos_t *pos, __unused char *arg)
{
    pos_startpos(pos);
    return 1;
}

int do_pos(pos_t *pos, __unused char *arg)
{
    pos_print(pos);
    return 1;
}

int do_genmoves(pos_t *pos, __unused char *arg)
{
    moves_gen_all(pos);
    return 1;
}

int do_prmoves(pos_t *pos, __unused char *arg)
{
    uint debug_level = debug_level_get();
    debug_level_set(1);
    moves_print(pos, M_PR_SEPARATE | M_PR_NUM | M_PR_LONG);
    debug_level_set(debug_level);
    return 1;
}

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

int do_prpieces(pos_t *pos, __unused char *arg)
{
    log_f(1, "%s\n", arg);
    pos_pieces_print(pos);
    return 1;
}

int do_memstats(__unused pos_t *pos,__unused char *arg)
{
    moves_pool_stats();
    piece_pool_stats();
    pos_pool_stats();
    return 1;
}

int do_move(__unused pos_t *pos, __unused char *arg)
{
    int i = 1, nmove = atoi(arg);
    move_t *move;
    pos_t *newpos;

    if (list_empty(&pos->moves[pos->turn])) {
        log_f(1, "No moves list.\n");
        return 0;
    }
    list_for_each_entry(move, &pos->moves[pos->turn], list) {
        if (i == nmove)
            goto doit;
        i++;
    }
    log_f(1, "Invalid <%d> move, should be <1-%d>.\n", nmove, i);
    return 0;
doit:
    newpos = move_do(pos, move);
    pos_print(newpos);

    return 1;
}

int do_quit(__unused pos_t *pos, __unused char *arg)
{
    return done = 1;
}

int do_debug(__unused pos_t *pos, __unused char *arg)
{
    debug_level_set(atoi(arg));
    return 1;
}

/* Print out help for ARG, or for all of the commands if ARG is
   not present. */
int do_help(__unused pos_t *pos, __unused char *arg)
{
    int i;
    int printed = 0;

    for (i = 0; commands[i].name; i++) {
        if (!*arg || (strcmp(arg, commands[i].name) == 0)) {
            printf("%-11.11s%s.\n", commands[i].name, commands[i].doc);
            printed++;
        }
    }

    if (!printed) {
        printf("No commands match `%s'.  Possibilties are:\n", arg);

        for (i = 0; commands[i].name; i++) {
            /* Print in six columns. */
            if (printed == 6) {
                printed = 0;
                printf("\n");
            }

            printf("%s\t", commands[i].name);
            printed++;
        }

        if (printed)
            printf("\n");
    }
    return 0;
}

int do_depth(__unused pos_t *pos, char *arg)
{
    depth = atoi(arg);
    printf("depth = %d\n", depth);
    return 1;

}

int do_search(pos_t *pos, __unused char *arg)
{
    int debug_level = debug_level_get();
    long long timer1, timer2;
    float nodes_sec;

    timer1 = debug_timer_elapsed();
    negamax(pos, depth, pos->turn == WHITE ? 1 : -1);
    timer2 = debug_timer_elapsed();
    nodes_sec = (float) pos->node_count / ((float) (timer2 - timer1) / (float)NANOSEC);
    debug_level_set(1);
    log(1, "best=");
    move_print(0, pos->bestmove, 0);
    log(1, " negamax=%d\n", pos->bestmove->negamax);
    debug_level_set(debug_level);
    printf("Total nodes: %lu time=%lld %.0f nodes/sec\n",
           pos->node_count, timer2 - timer1, nodes_sec);
    return 1;
}

int do_pvs(pos_t *pos, __unused char *arg)
{
    int debug_level = debug_level_get();
    long long timer1, timer2;
    float nodes_sec;

    timer1 = debug_timer_elapsed();
    pvs(pos, depth, EVAL_MIN, EVAL_MAX, pos->turn == WHITE ? 1 : -1);
    timer2 = debug_timer_elapsed();
    nodes_sec = (float) pos->node_count / ((float) (timer2 - timer1) / (float)NANOSEC);
    debug_level_set(1);
    log(1, "best=");
    move_print(0, pos->bestmove, 0);
    log(1, " negamax=%d\n", pos->bestmove->negamax);
    debug_level_set(debug_level);
    printf("Total nodes: %lu time=%lld %.0f nodes/sec\n",
           pos->node_count, timer2 - timer1, nodes_sec);
    return 1;
}

#ifdef BIN_brchess
/** main()
 * options:
 int brchess(pos_t *pos)
 *
 */
static int usage(char *prg)
{
    fprintf(stderr, "Usage: %s [-ilw] [file...]\n", prg);
    return 1;
}

#include <unistd.h>

int main(int ac, char **av)
{
    pos_t *pos;
    int opt;

    piece_pool_init();
    moves_pool_init();
    pos_pool_init();
    pos = pos_get();
    debug_init(1, stderr, true);
    eval_simple_init();

    while ((opt = getopt(ac, av, "d:f:")) != -1) {
        switch (opt) {
            case 'd':
                debug_level_set(atoi(optarg));
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
#endif /* BIN_brchess */
