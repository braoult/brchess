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
#include <string.h>
#include <stdlib.h>
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
        printf("\n");
    }
}

pool_t *pool_init(const char *name, uint32_t growsize, size_t eltsize)
{
    pool_t *pool;

    printf("%s: name=[%s] growsize=%u eltsize=%lu\n",
           __func__, name, growsize, eltsize);
    /* we need at least this space in struct */
    if (eltsize < sizeof (struct list_head))
        return NULL;
    if ((pool = malloc(sizeof (*pool)))) {
        pool->name = strdup(name);
        pool->growsize = growsize;
        pool->eltsize = eltsize;
        pool->available = 0;
        pool->allocated = 0;
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
    return _pool_add(pool, elt);
}

static struct list_head *_pool_get(pool_t *pool)
{
    struct list_head *res = pool->head.next;
    pool->available--;
    list_del(res);
    // printf("%s: res=%p\n", __func__, (void *)res);
    return res;
}

void *pool_get(pool_t *pool)
{
    if (!pool)
        return NULL;
    if (!pool->available) {
        void *alloc = malloc(pool->eltsize * pool->growsize);
        void *cur;
        uint32_t i;
        printf("+++ %s [%s]: growing pool from %u to %u elements.\n", __func__,
               pool->name,
               pool->allocated,
               pool->allocated + pool->growsize);

        if (!alloc)
            return NULL;
        //printf("       (old=%u)\n", pool->allocated);
        pool->allocated += pool->growsize;
        //printf("       (new=%u)\n", pool->allocated);
        for (i = 0; i < pool->growsize; ++i) {
            cur = alloc + i * pool->eltsize;
            //printf("%s: alloc=%p cur=%p\n", __func__, alloc, cur);
            _pool_add(pool, (struct list_head *)cur);
        }
        pool_stats(pool);
    }
    //printf("%s: returning %p pointer\n", __func__, res);
    return  _pool_get(pool);
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

    if ((pool = pool_init("dummy", 3, sizeof(*elt)))) {
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
