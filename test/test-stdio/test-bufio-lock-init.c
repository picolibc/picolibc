/*
Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
SPDX-License-Identifier: BSD-3-Clause-Clear

Redistribution and use in source and binary forms, with or without
modification, are permitted (subject to the limitations in the
disclaimer below) provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following
    disclaimer in the documentation and/or other materials provided
    with the distribution.

  * Neither the name of Qualcomm Technologies, Inc. nor the names of its
    contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
 * Regression test for the __bufio_lock lazy-init race.
 *
 * When multiple threads call fprintf on a FILE whose bf->lock is still
 * NULL, the unprotected "if (!bf->lock) __lock_init(bf->lock)" in
 * __bufio_lock allows two threads to each allocate a separate mutex slot
 * and the second writer silently overwrites bf->lock.  The first thread
 * then releases a mutex it never acquired, which is undefined behaviour
 * in any conforming lock implementation.
 *
 * The fix delegates to __bufio_lock_init(), which performs double-checked
 * locking under __LIBC_LOCK() so only one slot is ever allocated.
 *
 * This test opens a fresh FILE (bf->lock == NULL), lines up NUM_THREADS
 * threads on a barrier so they all enter fprintf simultaneously, and
 * verifies the total byte count of the output.  Without the fix,
 * assertions in native-locks.c abort the process when the race fires;
 * with the fix it passes cleanly.
 */

#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef TEST_FILE_NAME
#define TEST_FILE_NAME "BUFLK.TXT"
#endif

#define NUM_THREADS 16
#define ITERS       20
#define STRING      "hello, world\n"

/*
 * Threading primitives provided by native-locks.c (compiled separately,
 * without picolibc headers, so <pthread.h> lives there and not here).
 *
 * start_threads(n, func, arg) — spawn n threads running func(arg), each
 *   of which must call sync_threads() to wait at the shared barrier.
 *   The barrier count is n+1, so the main thread must also call
 *   sync_threads() once to release everyone simultaneously.
 * stop_threads()  — join all n threads.
 * sync_threads()  — barrier wait; blocks until all n+1 participants arrive.
 */
int  start_threads(int n, void *(*func)(void *), void *arg);
int  stop_threads(void);
void sync_threads(void);

#ifdef NO_NEWLIB
/*
 * When building against the native system libc (NO_NEWLIB), <pthread.h>
 * is safe to include directly.  Provide inline implementations of the
 * three threading primitives so the -native link needs no extra library.
 * When building against picolibc, these symbols are resolved from
 * native-locks.c which is compiled separately without picolibc headers.
 */
#include <pthread.h>

#define MAX_THREADS 64
static pthread_t         no_newlib_threads[MAX_THREADS];
static int               no_newlib_nthreads;
static pthread_barrier_t no_newlib_barrier;

int
start_threads(int n, void *(*func)(void *), void *arg)
{
    int i;

    no_newlib_nthreads = n;
    pthread_barrier_init(&no_newlib_barrier, NULL, (unsigned)(n + 1));
    for (i = 0; i < n; i++) {
        int ret = pthread_create(&no_newlib_threads[i], NULL, func, arg);
        if (ret)
            return ret;
    }
    return 0;
}

int
stop_threads(void)
{
    int i;

    for (i = 0; i < no_newlib_nthreads; i++)
        pthread_join(no_newlib_threads[i], NULL);
    pthread_barrier_destroy(&no_newlib_barrier);
    return 0;
}

void
sync_threads(void)
{
    pthread_barrier_wait(&no_newlib_barrier);
}
#endif /* NO_NEWLIB */

static FILE *out;

static void *
thread_func(void *arg)
{
    int i;

    (void)arg;

    /*
     * Hold here until every thread (and main) has called sync_threads(),
     * so all NUM_THREADS threads enter fprintf at the same instant.
     * This maximises the chance that multiple threads race through the
     * "if (!bf->lock)" check before any one of them finishes init.
     */
    sync_threads();

    for (i = 0; i < ITERS; i++)
        fprintf(out, "%s", STRING);

    return NULL;
}

int
main(void)
{
    FILE *in;
    int   status = 0;

#ifdef __SINGLE_THREAD
    printf("Single thread mode, test skipped\n");
    return 77;
#endif

    /* Open a fresh file so bf->lock is guaranteed NULL on first use. */
    out = fopen(TEST_FILE_NAME, "w");
    if (!out) {
        perror(TEST_FILE_NAME);
        return 1;
    }

    /*
     * Spawn all NUM_THREADS workers.  start_threads initialises the
     * barrier to NUM_THREADS+1 (workers + main), so main must call
     * sync_threads() once to release everyone simultaneously.
     */
    if (start_threads(NUM_THREADS, thread_func, NULL) != 0) {
        perror("start_threads");
        return 1;
    }

    /* Main thread releases the barrier so all workers start together. */
    sync_threads();

    stop_threads();
    fclose(out);

    /*
     * Verify total bytes written equals NUM_THREADS * ITERS * strlen(STRING).
     * Individual lines may be interleaved under __STDIO_BUFIO_LOCKING
     * (expected without flockfile), but every byte must be present.
     * A crash from the lazy-init race would have aborted before this point.
     */
    in = fopen(TEST_FILE_NAME, "r");
    if (!in) {
        perror(TEST_FILE_NAME);
        return 1;
    }
    fseek(in, 0, SEEK_END);
    {
        long expected = (long)(NUM_THREADS * ITERS * strlen(STRING));
        long actual = ftell(in);
        if (actual != expected) {
            printf("byte count wrong: expected %ld got %ld\n", expected, actual);
            status = 1;
        }
    }
    fclose(in);
    remove(TEST_FILE_NAME);

    return status;
}
