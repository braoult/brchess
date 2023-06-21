/* bodichess.c - main loop.
 *
 * Copyright (C) 2021 Bruno Raoult ("br")
 * Licensed under the GNU General Public License v3.0 or later.
 * Some rights reserved. See COPYING.
 *
 * You should have received a copy of the GNU General Public License along with this
 * program. If not, see <https://www.gnu.org/licenses/gpl-3.0-standalone.htmlL>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later <https://spdx.org/licenses/GPL-3.0-or-later.html>
 *
 */


#include <stdio.h>
#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "chessdefs.h"
#include "board.h"
#include "piece.h"
#include "move.h"
#include "list.h"
#include "debug.h"
#include "fen.h"
#include "eval.h"
#include "bodichess.h"

typedef struct {
    char *name;                                   /* User printable name */
    int (*func)(pos_t *, char *);              /* function doing the job */
    char *doc;                                    /* function doc */
} COMMAND;

/* readline example inspired by :
 * - https://thoughtbot.com/blog/tab-completion-in-gnu-readline
 * - http://web.mit.edu/gnu/doc/html/rlman_2.html
 */
char **commands_completion(const char *, int, int);
char *commands_generator(const char *, int);
char *escape(const char *);
int quote_detector(char *, int);
int execute_line (pos_t *, char *line);
COMMAND *find_command (char *);
char *stripwhite (char *string);

/* The names of functions that actually do the manipulation. */
int do_help(pos_t *, char*);
int do_fen(pos_t *, char*);
int do_pos(pos_t *, char*);
int do_genmoves(pos_t *, char*);
int do_prmoves(pos_t *, char*);
int do_prmovepos(pos_t *pos, char *arg);
int do_prpieces(pos_t *pos, char *arg);
int do_memstats(pos_t *, char*);
int do_eval(pos_t *, char*);
int do_quit(pos_t *, char*);
int do_debug(pos_t *, char*);

COMMAND commands[] = {
    { "help", do_help, "Display this text" },
    { "?", do_help, "Synonym for 'help'" },
    { "fen", do_fen, "Set position to FEN" },
    { "pos", do_pos, "Print current position" },
    { "quit", do_quit, "Quit" },
    { "genmove", do_genmoves, "Generate next move list" },
    { "prmoves", do_prmoves, "Print position move list" },
    { "prmovepos", do_prmovepos, "Print Nth move resulting position" },
    { "prpieces", do_prpieces, "Print Pieces (from pieces lists)" },
    { "memstats", do_memstats, "Generate next move list" },
    { "eval", do_eval, "Eval current position" },
    { "debug", do_debug, "Set log level to LEVEL" },
    { NULL, (int(*)()) NULL, NULL }
};

static int done=0;

int bodichess(pos_t *pos)
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
char **commands_completion(const char *text,
                           __attribute__((unused)) int start,
                           __attribute__((unused)) int end)
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
    COMMAND *command;
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
COMMAND *find_command(char *name)
{
    register int i;

    for (i = 0; commands[i].name; i++)
        if (strcmp(name, commands[i].name) == 0)
            return &commands[i];

    return (COMMAND *)NULL;
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

int do_eval(__attribute__((unused)) pos_t *pos,
            __attribute__((unused)) char *arg)
{
    eval_t res = eval(pos);
    printf("eval=%ld (%.3f pawns)\n", res, (float)res/100);
    return 1;
}

int do_fen(pos_t *pos, char *arg)
{
    log_f(1, "%s\n", arg);
    fen2pos(pos, arg);
    return 1;
}

int do_pos(pos_t *pos,
            __attribute__((unused)) char *arg)
{
    log_f(1, "%s\n", arg);
    pos_print(pos);
    return 1;
}

int do_genmoves(pos_t *pos,
                __attribute__((unused)) char *arg)
{
    log_f(1, "%s\n", arg);
    moves_gen(pos, OPPONENT(pos->turn), false);
    moves_gen(pos, pos->turn, true);
    return 1;
}

int do_prmoves(pos_t *pos,
               __attribute__((unused)) char *arg)
{
    log_f(1, "%s\n", arg);
    moves_print(pos, M_PR_SEPARATE);
    return 1;
}

int do_prmovepos(pos_t *pos, char *arg)
{
    struct list_head *p_cur, *tmp;
    int movenum = atoi(arg), cur = 0;             /* starts with 0 */
    move_t *move;

    log_f(1, "%s\n", arg);
    list_for_each_safe(p_cur, tmp, &pos->moves) {
        move = list_entry(p_cur, move_t, list);
        if (cur++ == movenum)
            break;
    }
    pos_print(move->newpos);
    return 1;
}

int do_prpieces(pos_t *pos, __attribute__((unused)) char *arg)
{
    log_f(1, "%s\n", arg);
    pos_pieces_print(pos);
    return 1;
}

int do_memstats(__attribute__((unused)) pos_t *pos,
               __attribute__((unused)) char *arg)
{
    moves_pool_stats();
    piece_pool_stats();
    pos_pool_stats();
    return 1;
}

int do_quit(__attribute__((unused)) pos_t *pos,
            __attribute__((unused)) char *arg)
{
    return done = 1;
}

int do_debug(__attribute__((unused)) pos_t *pos,
            __attribute__((unused)) char *arg)
{
    debug_level_set(atoi(arg));
    return 1;
}

/* Print out help for ARG, or for all of the commands if ARG is
   not present. */
int do_help(__attribute__((unused)) pos_t *pos,
            __attribute__((unused)) char *arg)
{
    register int i;
    int printed = 0;

    for (i = 0; commands[i].name; i++) {
        if (!*arg || (strcmp(arg, commands[i].name) == 0)) {
            printf("%s\t\t%s.\n", commands[i].name, commands[i].doc);
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

#ifdef BIN_bodichess
/** main()
 * options:
 int bodichess(pos_t *pos)
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

    debug_init(1);
    piece_pool_init();
    moves_pool_init();
    pos_pool_init();
    pos = pos_get();

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
    printf("optind = %d ac = %d\n", optind, ac);
    if (optind < ac)
        return usage(*av);

    return bodichess(pos);
}
#endif /* BIN_bodichess */
