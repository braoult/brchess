/* thread.h - thread management.
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

#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>

#include <brlib.h>

#include "position.h"

#define MIN_THRDS 1
#define MAX_THRDS 16

typedef enum {
    THRD_DEAD,
    THRD_IDLE,
    THRD_WORKING,
} thread_status_t;

typedef enum {
    /* main thread to subs */
    THRD_DO_SEARCH,
    THRD_DO_STOP,
    THRD_DO_QUIT,
} thread_cmd_t;

typedef struct {
    int id;
    thread_status_t status;
    thread_cmd_t cmd;
    int fd[2];
    pos_t pos;
} thread_t;

typedef struct {
    int nb;
    thread_t thread[MAX_THRDS + 1];
} thread_pool_t;

int thrd_create(__unused int num);
int thread_init(int nb);

#endif  /* THREAD_H */
