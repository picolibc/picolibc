/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 Kees Cook
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

#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define check(condition, message)                    \
    do {                                             \
        if (!(condition)) {                          \
            printf("%s: %s\n", message, #condition); \
            exit(1);                                 \
        }                                            \
    } while (0)

static const struct {
    clockid_t   id;
    const char *name;
} clocks[] = {
    { CLOCK_REALTIME,           "CLOCK_REALTIME"           },
    { CLOCK_MONOTONIC,          "CLOCK_MONOTONIC"          },
    { CLOCK_PROCESS_CPUTIME_ID, "CLOCK_PROCESS_CPUTIME_ID" },
    { CLOCK_THREAD_CPUTIME_ID,  "CLOCK_THREAD_CPUTIME_ID"  },
};

static const struct {
    int       base;
    clockid_t id;
} bases[] = {
    { TIME_UTC,           CLOCK_REALTIME           },
    { TIME_MONOTONIC,     CLOCK_MONOTONIC          },
    { TIME_ACTIVE,        CLOCK_PROCESS_CPUTIME_ID },
    { TIME_THREAD_ACTIVE, CLOCK_THREAD_CPUTIME_ID  },
};

#define NCLOCKS (sizeof(clocks) / sizeof(clocks[0]))
#define NBASES  (sizeof(bases) / sizeof(bases[0]))

int
main(void)
{
    struct timespec ts, ts2;
    unsigned        i;

    for (i = 0; i < NCLOCKS; i++) {
        ts.tv_sec = -1;
        ts.tv_nsec = -1;
        check(clock_getres(clocks[i].id, &ts) == 0, clocks[i].name);
        check(ts.tv_sec >= 0, clocks[i].name);
        check(ts.tv_nsec >= 0 && ts.tv_nsec < 1000000000L, clocks[i].name);
        check(ts.tv_sec != 0 || ts.tv_nsec != 0, clocks[i].name);
        printf("%s resolution %lld.%09ld\n", clocks[i].name, (long long)ts.tv_sec,
               (long)ts.tv_nsec);

        /* POSIX allows a NULL ts */
        check(clock_getres(clocks[i].id, NULL) == 0, clocks[i].name);
    }

    errno = 0;
    check(clock_getres((clockid_t)1000, &ts) == -1, "invalid clock");
    check(errno == EINVAL, "invalid clock");

    for (i = 0; i < NBASES; i++) {
        check(timespec_getres(&ts, bases[i].base) == bases[i].base, "timespec_getres");
        check(clock_getres(bases[i].id, &ts2) == 0, "timespec_getres");
        check(ts.tv_sec == ts2.tv_sec && ts.tv_nsec == ts2.tv_nsec, "timespec_getres");
    }

    check(timespec_getres(&ts, 0) == 0, "timespec_getres invalid base");

    printf("test passed\n");
    exit(0);
}
