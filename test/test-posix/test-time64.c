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
 * Times beyond 32 bits must reach the kernel intact on 32-bit
 * architectures, and kernel structures must be converted, not copied.
 */

#define _DEFAULT_SOURCE
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define check(condition, message)                      \
    do {                                               \
        if (!(condition)) {                            \
            printf("%s: %s\n", (message), #condition); \
            exit(1);                                   \
        }                                              \
    } while (0)

/* A sleep longer than 2^31 seconds, so a 32-bit time can't hold it */
#define LONG_SLEEP ((time_t)INT32_MAX + 100)

static void
on_alarm(int sig)
{
    (void)sig;
}

static void
check_long_sleeps(void)
{
    struct timespec req, rem;
    int             ret;

    if (sizeof(time_t) < 8)
        return;

    req.tv_sec = LONG_SLEEP;
    req.tv_nsec = 0;

    alarm(1);
    errno = 0;
    ret = nanosleep(&req, &rem);
    check(ret == -1 && errno == EINTR, "nanosleep interrupted");
    check(rem.tv_sec > INT32_MAX, "nanosleep remaining time beyond 32 bits");

    alarm(1);
    ret = clock_nanosleep(CLOCK_REALTIME, 0, &req, &rem);
    check(ret == EINTR, "clock_nanosleep interrupted");
    check(rem.tv_sec > INT32_MAX, "clock_nanosleep remaining time beyond 32 bits");
}

static void
check_itimers(void)
{
#ifdef __PICOLIBC__
    unsigned int left;

    /* alarm can't fail; a huge request must stay huge, not wrap */
    alarm(UINT_MAX);
    left = alarm(0);
    check(left > 0x7fff0000u, "alarm(UINT_MAX) left a long alarm");

    /* setitimer clamps seconds the kernel's long can't hold, rather than wrapping */
#if LONG_MAX < INT64_MAX
    {
        struct itimerval itv = { 0 }, cur;

        itv.it_value.tv_sec = (time_t)LONG_MAX + 1;
        itv.it_interval.tv_sec = (time_t)LONG_MAX * 4;
        check(setitimer(ITIMER_REAL, &itv, NULL) == 0, "setitimer beyond the kernel's range");
        check(getitimer(ITIMER_REAL, &cur) == 0, "getitimer");
        check(cur.it_value.tv_sec > LONG_MAX - 60 && cur.it_value.tv_sec <= LONG_MAX,
              "clamped value");
        check(cur.it_interval.tv_sec == LONG_MAX, "clamped interval");
        memset(&itv, 0, sizeof(itv));
        check(setitimer(ITIMER_REAL, &itv, NULL) == 0, "disarm");
    }
#endif
#endif
}

static void
check_wait3_rusage(void)
{
    struct rusage   ru;
    struct timespec start, now;
    int             status;
    pid_t           pid;

    pid = fork();
    check(pid >= 0, "fork");
    if (pid == 0) {
        /* Use about 0.2 s of CPU time */
        clock_gettime(CLOCK_MONOTONIC, &start);
        do {
            clock_gettime(CLOCK_MONOTONIC, &now);
        } while ((now.tv_sec - start.tv_sec) * 1000000000L + (now.tv_nsec - start.tv_nsec)
                 < 200000000L);
        _exit(0);
    }
    check(wait3(&status, 0, &ru) == pid, "wait3");
    check(WIFEXITED(status) && WEXITSTATUS(status) == 0, "child status");
    check(ru.ru_utime.tv_sec >= 0 && ru.ru_utime.tv_sec < 100, "ru_utime seconds");
    check(ru.ru_utime.tv_usec >= 0 && ru.ru_utime.tv_usec < 1000000, "ru_utime microseconds");
    check(ru.ru_stime.tv_sec >= 0 && ru.ru_stime.tv_sec < 100, "ru_stime seconds");
    check(ru.ru_stime.tv_usec >= 0 && ru.ru_stime.tv_usec < 1000000, "ru_stime microseconds");
    check(ru.ru_utime.tv_sec + ru.ru_utime.tv_usec + ru.ru_stime.tv_sec + ru.ru_stime.tv_usec > 0,
          "child used some CPU time");
}

int
main(void)
{
    check(signal(SIGALRM, on_alarm) != SIG_ERR, "signal");
    check_long_sleeps();
    check_itimers();
    check_wait3_rusage();
    printf("success\n");
    return 0;
}
