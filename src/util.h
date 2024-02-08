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

#undef safe_malloc
#undef safe_free


#define safe_malloc(size) ({                   \
            void *_ret = malloc(size);         \
            bug_on(_ret == NULL);              \
            _ret;                              \
        })

#define safe_free(ptr) do {                    \
        bug_on(ptr == NULL);                   \
        free(_ret);                            \
    } while (0)

#endif  /* UTIL_H */
