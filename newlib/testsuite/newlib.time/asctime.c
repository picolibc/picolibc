/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2023 Keith Packard
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

#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>

static struct {
    struct tm   tm;
    char        *result;
    int         _errno;
} tests[] = {
    {
        .tm = {
            .tm_sec = 0,
            .tm_min = 0,
            .tm_hour = 0,
            .tm_mday = 0,
            .tm_mon = 0,
            .tm_year = 0,
            .tm_wday = 0,
            .tm_yday = 0,
            .tm_isdst = 0,
        },
        .result = "Sun Jan  0 00:00:00 1900\n",
        ._errno = 0,
    },
    {
        .tm = {
            .tm_sec = 0,
            .tm_min = 0,
            .tm_hour = 0,
            .tm_mday = 0,
            .tm_mon = 0,
            .tm_year = 8100,
            .tm_wday = 0,
            .tm_yday = 0,
            .tm_isdst = 0,
        },
        .result = NULL,
        ._errno = EOVERFLOW,
    },
    {
	/* winter time is March, 21st 2022 at 8:15pm and 20 seconds */
        .tm = {
            .tm_sec     = 20,
            .tm_min     = 15,
            .tm_hour    = 20,
            .tm_mday    = 21,
            .tm_mon     = 3 - 1,
            .tm_year    = 2022 - 1900,
            .tm_isdst   = 0
        },
        .result = "Sun Mar 21 20:15:20 2022\n",
        ._errno = 0,
    },
    {
        /* summer time is July, 15th 2022 at 10:50am and 40 seconds */
        .tm = {
            .tm_sec     = 40,
            .tm_min     = 50,
            .tm_hour    = 10,
            .tm_mday    = 15,
            .tm_mon     = 7 - 1,
            .tm_year    = 2022 - 1900,
            .tm_isdst   = 1
        },
        .result = "Sun Jul 15 10:50:40 2022\n",
        ._errno = 0,
    },
};

static int mylen(const char *s)
{
    if (s == NULL)
        return -1;
    return strlen(s);
}

int main(void)
{
    unsigned n;
    int err = 0;
    char buf[26];

    (void) tests;
    for (n = 0; n < sizeof(tests)/sizeof(tests[0]); n++)
    {
        const char *result = asctime_r(&tests[n].tm, buf);

        if (tests[n].result == NULL && result == NULL)
            continue;

        if (tests[n].result != NULL && result != NULL &&
            strcmp(tests[n].result, result) == 0)
        {
            continue;
        }
        printf("expect \"%s\"(%d) result \"%s\"(%d)\n", tests[n].result, mylen(tests[n].result), result, mylen(result));
        err = 1;
    }
    return err;
}
