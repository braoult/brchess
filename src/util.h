/* util.h - various util functions.
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

#ifndef _UTIL_H
#define _UTIL_H

#include <stdio.h>
#include <stdlib.h>

#include "bug.h"

#include "chessdefs.h"

/*
#define bug_on(expr) do {                                            \
        if (unlikely(expr)) {                                                  \
            fprintf(stderr,                                                    \
                    "** BUG IN %s[%s:%d]: assertion \"" #expr "\" failed.\n",  \
                    __func__, __FILE__,__LINE__);                              \
            abort();                                                           \
        }                                                                      \
    } while (0)

#define warn_on(expr) ({                                                           \
            int _ret = !!(expr);                                                   \
            if (unlikely(_ret)) {                                                  \
                fprintf(stderr,                                                    \
                        "** WARN ON %s[%s:%d]: assertion \"" #expr "\" failed.\n", \
                        __func__, __FILE__,__LINE__);                              \
            }                                                                      \
            _ret;                                                                  \
        })
*/
#undef safe_malloc
#define safe_malloc(size) ({                   \
            void *_ret = malloc(size);         \
            bug_on(_ret == NULL);              \
            _ret;                              \
        })

#undef safe_free
#define safe_free(ptr) do {                    \
        bug_on(ptr == NULL);                   \
        free(_ret);                            \
    } while (0)


void raw_bitboard_print(const bitboard bitboard, const char *title);

#endif  /* UTIL_H */
