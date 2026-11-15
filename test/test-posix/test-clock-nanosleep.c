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

#define _GNU_SOURCE /* for CLOCK_MONOTONIC until it is visible for POSIX.1-2008 */
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

/* clock_nanosleep returns errors and reports the time left when interrupted */

#define check(condition, message)                      \
    do {                                               \
        if (!(condition)) {                            \
            printf("%s: %s\n", (message), #condition); \
            exit(1);                                   \
        }                                              \
    } while (0)

static void
handler(int sig)
{
    (void)sig;
}

int
main(void)
{
    struct sigaction act;
    struct itimerval itv;
    struct timespec  ts, rem;
    int              save_errno, ret;

    /* Errors come back as the return value, leaving errno alone */
    errno = 0;
    save_errno = errno;
    ts.tv_sec = 0;
    ts.tv_nsec = 1000000000L;
    check(clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL) == EINVAL, "bad time");
    ts.tv_nsec = 1000;
    check(clock_nanosleep((clockid_t)-1, 0, &ts, NULL) == EINVAL, "bad clock");
    check(errno == save_errno, "errno unchanged");
    check(clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL) == 0, "short sleep");

    /* A signal interrupts a long sleep, which reports the time left */
    memset(&act, 0, sizeof(act));
    act.sa_handler = handler;
    sigemptyset(&act.sa_mask);
    check(sigaction(SIGALRM, &act, NULL) == 0, "sigaction");
    memset(&itv, 0, sizeof(itv));
    itv.it_value.tv_usec = 50000;
    check(setitimer(ITIMER_REAL, &itv, NULL) == 0, "setitimer");
    ts.tv_sec = 10;
    ts.tv_nsec = 0;
    memset(&rem, 0, sizeof(rem));
    ret = clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, &rem);
    check(ret == EINTR, "interrupted");
    check(errno == save_errno, "errno unchanged when interrupted");
    check(rem.tv_sec >= 8 && rem.tv_sec < 10, "time remaining");
    check(rem.tv_nsec >= 0 && rem.tv_nsec < 1000000000L, "remaining nanoseconds");

    printf("success\n");
    return 0;
}
