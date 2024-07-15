/* alloc.h - various memory allocation helpers
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

#ifndef _ALLOC_H
#define _ALLOC_H

#include "chessdefs.h"

void *alloc(size_t size);
void *alloc_aligned(size_t align, size_t size);
void *alloc_aligned_page(size_t size);
void *alloc_aligned_hugepage(size_t size);

void *safe_alloc(size_t size);
void *safe_alloc_aligned(size_t align, size_t size);
void *safe_alloc_aligned_page(size_t size);
void *safe_alloc_aligned_hugepage(size_t size);
void safe_free(void *ptr);

#endif /* _ALLOC_H */
