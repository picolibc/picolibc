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

#define _DEFAULT_SOURCE
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <signal.h>
#include <stdint.h>

#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 2) || __GNUC__ > 4)
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wuse-after-free"
#endif

static sigjmp_buf abort_buf;

static void
trap_abort(int signum)
{
    if (signum == SIGABRT)
        errno = ENOMEM;
    else
        errno = EINVAL;
    siglongjmp(abort_buf, 1);
}

int
main(void)
{
    static size_t size;
    static int    ret = 0;
    static void  *ptr;

    for (size = 1; size < 2048; size++) {
        ptr = malloc(size);
        if (!ptr) {
#if SIZE_MAX > 0x7fffffff
            ret = 1;
            printf("malloc failed at size %zd\n", size);
#endif
            continue;
        }
        memset(ptr, 'a', size);
        errno = 0;
        signal(SIGABRT, SIG_DFL);
        free(ptr);
        if (errno != 0) {
            int err = errno;
            ret = 1;
            printf("first free failed at size %zd with errno %d %s\n", size, err, strerror(err));
        }
        errno = 0;
        signal(SIGABRT, trap_abort);
        if (sigsetjmp(abort_buf, 1) == 0)
            free(ptr);
        if (errno != ENOMEM) {
            int err = errno;
            ret = 1;
            printf("second free failed at size %zd didn't set errno to ENOMEM. got %d %s\n", size,
                   err, strerror(err));
        }
#ifdef __PICOLIBC__
        /* glibc doesn't survive when realloc is called with a free block */
        ptr = malloc(size);
        if (!ptr) {
            ret = 1;
            printf("malloc failed at size %zd\n", size);
            continue;
        }
        memset(ptr, 'a', size);
        errno = 0;
        signal(SIGABRT, SIG_DFL);
        free(ptr);
        if (errno != 0) {
            int err = errno;
            ret = 1;
            printf("first free failed at size %zd with errno %d %s\n", size, err, strerror(err));
        }

        errno = 0;
        signal(SIGABRT, trap_abort);
        if (sigsetjmp(abort_buf, 1) == 0) {
            void *nptr;
            nptr = realloc(ptr, size + 10);
            if (nptr)
                memset(nptr, 'b', size + 10);
            free(nptr);
        }
        if (errno != ENOMEM) {
            int err = errno;
            ret = 1;
            printf("realloc failed at size %zd didn't set errno to ENOMEM. got %d %s\n", size, err,
                   strerror(err));
        }
#endif
    }
    return ret;
}
