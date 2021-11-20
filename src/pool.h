/* pool.h - A simple memory pool manager.
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

#ifndef POOL_H
#define POOL_H

#include <stdint.h>
#include <stddef.h>
#include "list.h"
#include "bits.h"

typedef struct {
    char *name;
    u32 available;
    u32 allocated;
    u32 growsize;
    size_t eltsize;
    struct list_head head;
} pool_t;

void pool_stats(pool_t *pool);
pool_t *pool_init(const char *name, u32 grow, size_t size);
void *pool_get(pool_t *pool);
u32 pool_add(pool_t *pool, void *elt);

#endif
