/* thread.c - thread management.
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

#include <brlib.h>
#include <pthread.h>
#include <poll.h>
#include <sys/socket.h>

#include "thread.h"

/* Still have to decide: thread or process ?
 *
 */
thread_pool_t threadpool;

/**
 * thrd_create - initialize thrd.
 */
int thrd_create(__unused int num)
{
    int fd[2];
    /* shall we make a communication channel via a pipe or socket ? */
    int __unused ret = socketpair(AF_LOCAL, SOCK_SEQPACKET, PF_LOCAL, fd);
    return 1;
}

/**
 * thread_init - initialize thread pool.
 */
int thread_init(int nb)
{
    nb = clamp(nb, MIN_THRDS, MAX_THRDS);

    /* stop unwanted threads, always keep 1 */
    for (int i = nb + 1; i < threadpool.nb; ++i) {
        printf("stopping thread %d - status = \n", i);
        threadpool.thread[i].cmd = THRD_DO_QUIT;
    }
    for (int i = threadpool.nb; i < nb; ++i) {
        printf("creating thread %d - status = \n", i);
        thrd_create(i);

    }

    return nb;
}

/*
  communication:
  main thread -> thread
           commands via memory
  thread -> main thread
           status via memory
           output via pipe/socket
  thread output will be output/filtered by main thread

 */
