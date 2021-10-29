/* pool.c - A simple pool manager.
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

/*
#include <stdbool.h>
#include <ctype.h>
*/

#include <stddef.h>
#include <malloc.h>
//extern char* strdup(const char*);
#include <string.h>
#include <stdlib.h>
//#include <stdio.h>

#include "list.h"
#include "pool.h"

void pool_stats(pool_t *pool)
{
    if (pool) {
        printf("[%s] pool [%p]: ", pool->name, (void *)pool);
        printf("avail:%u ", pool->available);
        printf("alloc:%u ", pool->allocated);
        printf("grow:%u ", pool->growsize);
        printf("eltsize:%lu ", pool->eltsize);
        printf("offset:%lu ", pool->list_offset);
        printf("\n");
    }
}

pool_t *pool_init(const char *name, uint32_t growsize, size_t eltsize,
                  ptrdiff_t list_offset)
{
    pool_t *pool;

    if ((pool = malloc(sizeof (*pool)))) {
        pool->name = strdup(name);
        pool->growsize = growsize;
        pool->eltsize = eltsize;
        pool->available = 0;
        pool->allocated = 0;
        pool->list_offset = list_offset;
        INIT_LIST_HEAD(&pool->head);
    }
    return pool;
}

static uint32_t _pool_add(pool_t *pool, struct list_head *elt)
{
    /*
    printf("%s: pool=%p &head=%p elt=%p off1=%lu off2=%lu\n",
           __func__,
           (void *)pool,
           (void *)&pool->head,
           (void *)elt,
           (void *)&pool->head-(void *)pool,
           offsetof(pool_t, head));
    */

    list_add(elt, &pool->head);
    return ++pool->available;
}

uint32_t pool_add(pool_t *pool, void *elt)
{
    struct list_head *plist = elt + pool->list_offset;;
    return _pool_add(pool, plist);
}

static struct list_head *_pool_get(pool_t *pool)
{
    struct list_head *res = pool->head.next;
    pool->available--;
    list_del(res);
    //printf("%s: res=%p\n", __func__, (void *)res);
    return res;
}

void *pool_get(pool_t *pool)
{
    if (!pool)
        return NULL;
    if (!pool->available) {
        printf("+++ %s [%s]: allocating new pool (old=%u)\n", __func__, pool->name,
               pool->allocated);
        void *alloc = malloc(pool->eltsize * pool->growsize);
        void *cur;
        struct list_head *plist;
        uint32_t i;

        if (!alloc)
            return NULL;
        pool->allocated += pool->growsize;
        //printf("       (old=%u)\n", pool->allocated);
        for (i = 0; i < pool->growsize; ++i) {
            cur = alloc + i * pool->eltsize;
            plist = cur + pool->list_offset;
            printf("%s: alloc=%p cur=%p plist=%p off=%lu\n", __func__,
                   alloc, cur, (void *)plist, (void *)plist-cur);
            _pool_add(pool, plist);
        }
    }
    //list_del(pool->head.next);
    void *res = _pool_get(pool);
    printf("%s: returning %p pointer\n", __func__, res);
    return res - pool->list_offset;
}

#ifdef POOLBIN
struct d {
    uint16_t data1;
    char c;
    struct list_head list;
};

static LIST_HEAD (head);

int main(int ac, char**av)
{
    pool_t *pool;
    int total;
    int action=0;
    uint16_t icur=0;
    char ccur='z';
    struct d *elt;

    printf("%s: sizeof(d)=%lu sizeof(*d)=%lu off=%lu\n", *av, sizeof(elt),
           sizeof(*elt), offsetof(struct d, list));

    if ((pool = pool_init("dummy", 3, sizeof(*elt), offsetof(struct d, list)))) {
        pool_stats(pool);
        for (int cur=1; cur<ac; ++cur) {
            total = atoi(av[cur]);
            if (action == 0) {                    /* add elt to list */
                printf("adding %d elements\n", total);
                for (int i = 0; i < total; ++i) {
                    elt = pool_get(pool);
                    elt->data1 = icur++;
                    elt->c = ccur--;
                    list_add(&elt->list, &head);
                }
                pool_stats(pool);
                action = 1;
            } else {                              /* remove one elt from list */
                printf("deleting %d elements\n", total);
                for (int i = 0; i < total; ++i) {
                    if (!list_empty(&head)) {
                        elt = list_last_entry(&head, struct d, list);
                        printf("elt=[%d, %c]\n", elt->data1, elt->c);
                        list_del(&elt->list);
                        pool_add(pool, elt);
                    }
                }
                pool_stats(pool);
                action = 0;
            }
        }
    }
    pool_stats(pool);
}
#endif
