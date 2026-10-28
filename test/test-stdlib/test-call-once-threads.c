/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 Keith Packard
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

/*
 * Verify call_once in the presence of threads.
 *
 * For each round, NUM_THREADS threads are released together from a
 * barrier and all call call_once on a fresh flag. The test checks
 * that the function runs exactly once per flag, that it never runs
 * concurrently with itself, and that every thread observes the
 * effects of the function once call_once returns, even when it had
 * to wait for another thread to finish running it.
 *
 * The locking and threading primitives come from native-locks.c,
 * whose libc lock is an error-checking (non-recursive) mutex, so the
 * function passed to call_once must not use the C library.
 */

#define _ISOC23_SOURCE
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 16
#define ROUNDS      64
#define SPIN        10000

int                start_threads(int n, void *(*func)(void *), void *arg);
int                stop_threads(void);
void               sync_threads(void);

static once_flag   flags[ROUNDS];
static _Atomic int calls[ROUNDS];
static _Atomic int done[ROUNDS];

/* Written by main only between the end of one round and the start of the next */
static int         current_round;

static _Atomic int running;
static _Atomic int overlaps;
static _Atomic int failures;

static void
init(void)
{
    volatile int spin;
    int          round = current_round;

    if (atomic_fetch_add(&running, 1) != 0)
        atomic_fetch_add(&overlaps, 1);

    atomic_fetch_add(&calls[round], 1);

    /* Keep the other threads waiting in call_once for a while */
    for (spin = 0; spin < SPIN; spin++)
        ;

    atomic_store_explicit(&done[round], 1, memory_order_relaxed);
    atomic_fetch_sub(&running, 1);
}

static void *
thread_func(void *arg)
{
    int r;

    (void)arg;

    for (r = 0; r < ROUNDS; r++) {
        /* Start of round: all threads enter call_once together */
        sync_threads();

        call_once(&flags[r], init);

        /*
         * call_once must not return before init has completed, so
         * its effects must be visible here without further
         * synchronization.
         */
        if (atomic_load_explicit(&done[r], memory_order_relaxed) != 1
            || atomic_load_explicit(&calls[r], memory_order_relaxed) != 1)
            atomic_fetch_add(&failures, 1);

        /* End of round: lets main safely advance current_round */
        sync_threads();
    }
    return NULL;
}

int
main(void)
{
    int r;
    int status = 0;

#ifdef __SINGLE_THREAD
    printf("Single thread mode, test skipped\n");
    return 77;
#endif

    for (r = 0; r < ROUNDS; r++)
        flags[r] = ONCE_FLAG_INIT;

    if (start_threads(NUM_THREADS, thread_func, NULL) != 0) {
        perror("start_threads");
        return 1;
    }

    for (r = 0; r < ROUNDS; r++) {
        current_round = r;
        sync_threads();
        sync_threads();
    }

    stop_threads();

    for (r = 0; r < ROUNDS; r++) {
        if (calls[r] != 1) {
            printf("round %d: function called %d times\n", r, calls[r]);
            status = 1;
        }
    }
    if (overlaps != 0) {
        printf("function ran concurrently %d times\n", overlaps);
        status = 1;
    }
    if (failures != 0) {
        printf("%d calls returned before the function completed\n", failures);
        status = 1;
    }

    if (status == 0)
        printf("call_once thread tests passed\n");
    return status;
}
