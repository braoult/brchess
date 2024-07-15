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
#include <sys/mman.h>

#include <brlib.h>
#include <bitops.h>
#include <bug.h>

#include "chessdefs.h"
#include "alloc.h"

/* values for linux
 * PAGE_SIZE can be obtained with getpagesize(2), but unfortunately nothing for
 * HUGE_PAGE_SIZE (except looking in /proc/meminfo or such).
 */
#define PAGE_SIZE       (4 * 1024)                /* 4 Kb */
#define HUGE_PAGE_SIZE  (2 * 1024 * 1024)         /* 2 Mb */

/**
 * alloc() - allocate memory.
 * @size:  size to allocate
 *
 * Allocate memory on the heap.
 *
 * @return: memory address if success, NULL otherwise.
 */
void *alloc(size_t size)
{
    return malloc(size);
}

/**
 * alloc_aligned() - allocate aligned memory.
 * @align: alignment, in bytes
 * @size:  size to allocate
 *
 * Allocate aligned memory on the heap. @align must be a power of 2 and
 * a multiple of sizeof(void *). See aligned_alloc(3) for details.
 *
 * @return: memory address if success, NULL otherwise.
 */
void *alloc_aligned(size_t align, size_t size)
{
    bug_on(!size || !align ||
           align & (align - 1) || align % sizeof (void *));
    return aligned_alloc(align, size);
}

/**
 * alloc_page_aligned() - allocate page-aligned memory.
 * @size:  size to allocate
 *
 * Allocate page-aligned memory on the heap.
 *
 * @return: memory address if success, NULL otherwise.
 */
void *alloc_aligned_page(size_t size)
{
    /* round size (up) to alignment */
    //size_t rounded = (size + PAGE_SIZE - 1) & -PAGE_SIZE;
    void *mem = alloc_aligned(PAGE_SIZE, size);
    return mem;
}

/**
 * alloc_huge_page_aligned() - allocate huge-page-aligned memory.
 * @size:  size to allocate
 *
 * Allocate page-aligned memory on the heap.
 *
 * @return: memory address if success, NULL otherwise.
 */
void *alloc_aligned_hugepage(size_t size)
{
    /* round size (up) to alignment */
    size_t rounded = (size + PAGE_SIZE - 1) & -PAGE_SIZE;
    void *mem = alloc_aligned(PAGE_SIZE, rounded);
    printf("size=%zu rounded=%zu\n", size, rounded);
    //void *mem = aligned_alloc(HUGE_PAGE_SIZE, size);
    if (mem) {
        if (madvise(mem, rounded, MADV_HUGEPAGE | MADV_RANDOM))
            perror("madvise");
    }

    return mem;
}

/**
 * safe_alloc() - allocate memory or fail.
 * @size:  size to allocate
 *
 * Allocate memory on the heap. This function does not return if allocation
 * fails.
 *
 * @return: memory address (if success only).
 */
void *safe_alloc(size_t size)
{
    void *mem = malloc(size);
    bug_on_always(mem == NULL);
    return mem;
}

/**
 * safe_alloc_aligned() - allocate aligned memory or fail.
 * @align: alignment.
 * @size:  size to allocate
 *
 * Allocate aligned memory on the heap.
 * This function does not return if allocation fails. See alloc_aligned()
 * for more details.
 *
 * @return: memory address (if success only).
 */
void *safe_alloc_aligned(size_t align, size_t size)
{
    void *mem = alloc_aligned(align, size);
    bug_on_always(mem == NULL);
    return mem;
}

/**
 * safe_alloc_aligned_page() - allocate page-aligned memory or fail.
 * @size:  size to allocate
 *
 * Allocate memory on the heap. This function does not return if allocation
 * fails. See alloc_aligned() for more details.
 *
 * @return: memory address (if success only).
 */
void *safe_alloc_aligned_page(size_t size)
{
    void *mem = alloc_aligned_page(size);
    bug_on_always(mem == NULL);
    return mem;
}

/**
 * safe_alloc_aligned_hugepage() - allocate huge page aligned memory or fail.
 * @size:  size to allocate
 *
 * Allocate memory on the heap. This function does not return if allocation
 * fails. See alloc_aligned() for more details.
 *
 * @return: memory address (if success only).
 */
void *safe_alloc_aligned_hugepage(size_t size)
{
    /* round size (up) to alignment */
    //size_t rounded = (size + HUGE_PAGE_SIZE - 1) & -HUGE_PAGE_SIZE;
    //void *mem = aligned_alloc(rounded, HUGE_PAGE_SIZE);
    void *mem = alloc_aligned_hugepage(size);
    bug_on_always(mem == NULL);
    return mem;
}

void safe_free(void *ptr)
{
    bug_on_always(ptr == NULL);
    free(ptr);
}
