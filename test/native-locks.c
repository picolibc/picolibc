/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2022 Keith Packard
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define _DEFAULT_SOURCE
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>

/*
 * Validate lock usage in libc by creating fake locks
 * to be used during testing
 */

#define _LOCK_T struct __lock*

struct __lock {
    pthread_mutex_t     mut;
};

struct __lock __lock___libc_recursive_mutex;

__attribute__((constructor))
static void libc_lock_init(void)
{
        pthread_mutexattr_t     mutexattr;
        pthread_mutexattr_init(&mutexattr);
        pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);

        pthread_mutex_init(&__lock___libc_recursive_mutex.mut, &mutexattr);
}

#define MAX_LOCKS 32

static struct __lock locks[MAX_LOCKS];
static _Atomic int in_use[MAX_LOCKS];

/* Create a new dynamic non-recursive lock */
void __retarget_lock_init(_LOCK_T *lock);

void __retarget_lock_init(_LOCK_T *lock)
{
        int lock_id = 0;

        for (lock_id = 0; lock_id < MAX_LOCKS; lock_id++) {
                int expected = 0;
                int desired = 1;
                if (atomic_compare_exchange_strong(&in_use[lock_id], &expected, desired)) {
                        pthread_mutexattr_t     mutexattr;
                        pthread_mutexattr_init(&mutexattr);
                        pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);

                        pthread_mutex_init(&locks[lock_id].mut, &mutexattr);
                        *lock = &locks[lock_id];
                        return;
                }
        }
}

/* Create a new dynamic recursive lock */
void __retarget_lock_init_recursive(_LOCK_T *lock);

void __retarget_lock_init_recursive(_LOCK_T *lock)
{
        __retarget_lock_init(lock);
}

/* Close dynamic non-recursive lock */
void __retarget_lock_close(_LOCK_T lock);

void __retarget_lock_close(_LOCK_T lock)
{
        int lock_id = lock - locks;
        int desired = 0;

        pthread_mutex_destroy(&locks[lock_id].mut);

        atomic_exchange(&in_use[lock_id], desired);
}

/* Close dynamic recursive lock */
void __retarget_lock_close_recursive(_LOCK_T lock);

void __retarget_lock_close_recursive(_LOCK_T lock)
{
        __retarget_lock_close(lock);
}

/* Acquiure non-recursive lock */
void __retarget_lock_acquire(_LOCK_T lock);

void __retarget_lock_acquire(_LOCK_T lock)
{
        pthread_mutex_lock(&lock->mut);
}

/* Acquiure recursive lock */
void __retarget_lock_acquire_recursive(_LOCK_T lock);

void __retarget_lock_acquire_recursive(_LOCK_T lock)
{
        pthread_mutex_lock(&lock->mut);
}

/* Release non-recursive lock */
void __retarget_lock_release(_LOCK_T lock);

void __retarget_lock_release(_LOCK_T lock)
{
        pthread_mutex_unlock(&lock->mut);
}

/* Release recursive lock */
void __retarget_lock_release_recursive(_LOCK_T lock);

void __retarget_lock_release_recursive(_LOCK_T lock)
{
        pthread_mutex_unlock(&lock->mut);
}

static pthread_t thread;

int
start_thread(void *(*func)(void *), void *arg);

int
start_thread(void *(*func)(void *), void *arg)
{
        return pthread_create(&thread, NULL, func, arg);
}

int
stop_thread(void);

int
stop_thread(void)
{
        return pthread_join(thread, NULL);
}
