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

#define _ISOC23_SOURCE
#include <stdio.h>
#include <stdlib.h>

static once_flag static_flag = ONCE_FLAG_INIT;
static once_flag nested_flag = ONCE_FLAG_INIT;

static int       static_count;
static int       other_count;
static int       nested_count;

static void
count_static(void)
{
    static_count++;
}

static void
count_other(void)
{
    other_count++;
}

static void
count_nested(void)
{
    nested_count++;
    /* A nested call on the same flag must not invoke the function again */
    call_once(&nested_flag, count_nested);
}

static void
use_libc(void)
{
    /* The function may use other parts of the library */
    void *p = malloc(16);
    free(p);
    other_count++;
}

int
main(void)
{
    once_flag auto_flag = ONCE_FLAG_INIT;
    once_flag libc_flag = ONCE_FLAG_INIT;
    int       i;

    for (i = 0; i < 3; i++)
        call_once(&static_flag, count_static);
    if (static_count != 1) {
        printf("static flag: function called %d times\n", static_count);
        return 1;
    }

    for (i = 0; i < 3; i++)
        call_once(&auto_flag, count_other);
    if (other_count != 1) {
        printf("automatic flag: function called %d times\n", other_count);
        return 1;
    }

    /* A used flag stays used even when passed a different function */
    call_once(&static_flag, count_other);
    if (other_count != 1) {
        printf("used flag invoked a new function\n");
        return 1;
    }

    call_once(&nested_flag, count_nested);
    if (nested_count != 1) {
        printf("nested call: function called %d times\n", nested_count);
        return 1;
    }

    call_once(&libc_flag, use_libc);
    call_once(&libc_flag, use_libc);
    if (other_count != 2) {
        printf("libc flag: unexpected count %d\n", other_count);
        return 1;
    }

    printf("call_once tests passed\n");
    return 0;
}
