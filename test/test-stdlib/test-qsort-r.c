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
 * qsort_r accepts both the POSIX (GNU) argument order and, when BSD
 * interfaces are visible, the BSD order, chosen from the type of the
 * last argument.
 */

/* BSD and POSIX.1-2024 interfaces, unless the includer chose a level */
#ifndef _POSIX_C_SOURCE
#define _DEFAULT_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>

#define N 8

static const int sorted[N] = { -7, -1, 0, 2, 3, 5, 9, 11 };

static int       calls;

static void
fill(int *v)
{
    static const int init[N] = { 5, -1, 9, 0, 3, -7, 11, 2 };
    for (int i = 0; i < N; i++)
        v[i] = init[i];
}

static int
check(const char *name, const int *v)
{
    for (int i = 0; i < N; i++)
        if (v[i] != sorted[i]) {
            printf("%s: element %d is %d, expected %d\n", name, i, v[i], sorted[i]);
            return 1;
        }
    if (calls == 0) {
        printf("%s: comparison function never called\n", name);
        return 1;
    }
    return 0;
}

/* POSIX order: the thunk comes last */
static int
posix_cmp(const void *a, const void *b, void *thunk)
{
    *(int *)thunk += 1;
    return *(const int *)a - *(const int *)b;
}

static int
posix_cmp_nothunk(const void *a, const void *b, void *thunk)
{
    (void)thunk;
    calls++;
    return *(const int *)a - *(const int *)b;
}

#if __BSD_VISIBLE
/* BSD order: the thunk comes first */
static int
bsd_cmp(void *thunk, const void *a, const void *b)
{
    *(int *)thunk += 1;
    return *(const int *)a - *(const int *)b;
}

static int
bsd_cmp_nothunk(void *thunk, const void *a, const void *b)
{
    (void)thunk;
    calls++;
    return *(const int *)a - *(const int *)b;
}
#endif

#ifdef __cplusplus

#ifdef __clang__
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wmain"
#endif

extern "C" {

int main(void);
#endif

int
main(void)
{
    int   v[N];
    int   ret = 0;
    void *vcalls = &calls;

    fill(v);
    calls = 0;
    qsort_r(v, N, sizeof(v[0]), posix_cmp, &calls);
    ret |= check("POSIX order", v);

    fill(v);
    calls = 0;
    qsort_r(v, N, sizeof(v[0]), posix_cmp, vcalls);
    ret |= check("POSIX order, void * thunk", v);

    fill(v);
    calls = 0;
    qsort_r(v, N, sizeof(v[0]), posix_cmp_nothunk, NULL);
    ret |= check("POSIX order, NULL thunk", v);

    fill(v);
    calls = 0;
    (qsort_r)(v, N, sizeof(v[0]), posix_cmp, &calls);
    ret |= check("POSIX order, function", v);

#if __BSD_VISIBLE
    fill(v);
    calls = 0;
    qsort_r(v, N, sizeof(v[0]), &calls, bsd_cmp);
    ret |= check("BSD order", v);

    fill(v);
    calls = 0;
    qsort_r(v, N, sizeof(v[0]), vcalls, bsd_cmp);
    ret |= check("BSD order, void * thunk", v);

    fill(v);
    calls = 0;
    qsort_r(v, N, sizeof(v[0]), NULL, bsd_cmp_nothunk);
    ret |= check("BSD order, NULL thunk", v);
#endif

    if (ret == 0)
        printf("success\n");
    return ret;
}

#ifdef __cplusplus
}
#endif
