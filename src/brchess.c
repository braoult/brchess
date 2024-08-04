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
#include <unistd.h>

#include <brlib.h>

#include "position.h"
#include "fen.h"
#include "hist.h"
#include "uci.h"

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
    pos_t *pos;
    int opt;

    printf("brchess " VERSION "\n");
    init_all();
    pos = pos_new();
    printf("Engine ready.\n");

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

    return uci(pos);
}
