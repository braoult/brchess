/* alloc.c - various memory allocation helpers
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

#include <brlib.h>

#include "chessdefs.h"
#include "alloc.h"

/* force BUG_ON, to get a program abort for failed malloc/free
 */
#pragma push_macro("BUG_ON")
#undef BUG_ON
#define BUG_ON 1
#include <bug.h>

void *safe_alloc(size_t size)
{
    void *mem = malloc(size);
    bug_on(mem == NULL);
    return mem;
}

void *safe_alloc_page_aligned(size_t size)
{
    void *mem = malloc(size);
    bug_on(mem == NULL);
    return mem;
}

void safe_free(void *ptr)
{
    bug_on(ptr == NULL);
    free(ptr);
}

/* restore BUG_ON
 */
#pragma pop_macro("BUG_ON")
