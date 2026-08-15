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
#include <stdlib.h>
#include <stdio.h>

static size_t
random_int(size_t count)
{
    return ((size_t)random()) % count;
}

static void
shuffle(void **values, size_t n)
{
    size_t i;

    for (i = 0; i < n - 1; i++) {
        size_t p = random_int(n - i);
        void  *t = values[i];
        values[i] = values[i + p];
        values[i + p] = t;
    }
}

#if __SIZE_MAX__ < 0xffffffffUL
/*
 * Compute space for the pointer array and the objects on small
 * targets
 */
#define N ((__SIZE_MAX__) >> 4)
#else

#if defined(__MALLOC_SMALL_BUCKET) && __MALLOC_SMALL_BUCKET == 0
#define N ((size_t)1024 * (size_t)128)
#else
#define N ((size_t)1024 * (size_t)1024)
#endif

#endif

#define OBJ_SIZE 8

int
main(void)
{
    void **objs;
    size_t o;

    objs = calloc(N, sizeof(void *));
    if (!objs) {
        printf("cannot allocate objs array\n");
        return 77;
    }

    for (o = 0; o < N; o++) {
        objs[o] = malloc(OBJ_SIZE);
        if (!objs[o]) {
            printf("cannot allocate obj %zd\n", o);
            return 77;
        }
        if (o % 100000 == 0)
            printf("malloc %zd\n", o);
    }

    shuffle(objs, N);

    for (o = 0; o < N; o++) {
        free(objs[o]);
        if (o % 100000 == 0)
            printf("free %zd\n", o);
    }

    printf("All done\n");
    return 0;
}
