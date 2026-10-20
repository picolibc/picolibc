/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 Artem Kulyk
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
 * ftrylockfile and funlockfile must operate on the same lock.
 */

#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifndef TEST_FILE_NAME
#define TEST_FILE_NAME "FTRYLOCK.TXT"
#endif

int  start_threads(int n, void *(*func)(void *), void *arg);
int  stop_threads(void);
void sync_threads(void);

#ifndef __STDIO_LOCKING
int
main(void)
{
    return 77;
}
#else
static int worker_done;

static void *
getenv_worker(void *arg)
{
    (void)arg;
    sync_threads();
    (void)getenv("PATH");
    __atomic_store_n(&worker_done, 1, __ATOMIC_RELEASE);
    return NULL;
}

static int writer_done;

static void *
writer_worker(void *arg)
{
    FILE *f = arg;

    sync_threads();
    (void)fputs("x", f);
    __atomic_store_n(&writer_done, 1, __ATOMIC_RELEASE);
    return NULL;
}

static int
test_lock_target(FILE *f)
{
    if (ftrylockfile(f) != 0) {
        printf("ftrylockfile failed\n");
        return 1;
    }

    __atomic_store_n(&worker_done, 0, __ATOMIC_RELEASE);
    if (start_threads(1, getenv_worker, NULL) != 0) {
        printf("start_threads failed\n");
        return 2;
    }
    sync_threads();
    usleep(100000);
    if (!__atomic_load_n(&worker_done, __ATOMIC_ACQUIRE)) {
        printf("global libc lock leaked by ftrylockfile\n");
        return 1;
    }
    funlockfile(f);
    stop_threads();
    return 0;
}

static int
test_file_exclusion(FILE *f)
{
    if (ftrylockfile(f) != 0) {
        printf("ftrylockfile failed\n");
        return 1;
    }

    __atomic_store_n(&writer_done, 0, __ATOMIC_RELEASE);
    if (start_threads(1, writer_worker, f) != 0) {
        printf("start_threads failed\n");
        return 2;
    }
    sync_threads();
    usleep(100000);
    if (__atomic_load_n(&writer_done, __ATOMIC_ACQUIRE)) {
        printf("ftrylockfile did not exclude another writer\n");
        return 1;
    }
    funlockfile(f);
    stop_threads();
    if (!__atomic_load_n(&writer_done, __ATOMIC_ACQUIRE)) {
        printf("writer did not complete after funlockfile\n");
        return 1;
    }
    return 0;
}

int
main(void)
{
    FILE *f = fopen(TEST_FILE_NAME, "w");
    int   errors = 0;

    if (!f) {
        perror("fopen");
        return 2;
    }
    errors += test_lock_target(f);
    errors += test_file_exclusion(f);
    fclose(f);
    return errors ? 1 : 0;
}
#endif
