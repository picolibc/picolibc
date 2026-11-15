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

#define _POSIX_C_SOURCE 202405L
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* kill with signal 0 only checks the process; sigsuspend waits for a handler */

#define check(condition, message)                      \
    do {                                               \
        if (!(condition)) {                            \
            printf("%s: %s\n", (message), #condition); \
            exit(1);                                   \
        }                                              \
    } while (0)

static volatile sig_atomic_t caught;

static void
handler(int sig)
{
    caught = sig;
}

int
main(void)
{
    struct sigaction act;
    sigset_t         set, wait_mask, current;

    check(kill(getpid(), 0) == 0, "kill with signal 0");
    check(kill(getpid(), -1) == -1 && errno == EINVAL, "kill with a negative signal");

    memset(&act, 0, sizeof(act));
    act.sa_handler = handler;
    sigemptyset(&act.sa_mask);
    check(sigaction(SIGUSR1, &act, NULL) == 0, "sigaction");

    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    check(sigprocmask(SIG_BLOCK, &set, NULL) == 0, "sigprocmask");
    check(raise(SIGUSR1) == 0, "raise");
    check(caught == 0, "signal blocked");

    /* Waiting with SIGUSR1 unblocked runs the handler, then restores the mask */
    sigemptyset(&wait_mask);
    check(sigsuspend(&wait_mask) == -1 && errno == EINTR, "sigsuspend");
    check(caught == SIGUSR1, "handler ran during sigsuspend");
    check(sigprocmask(SIG_BLOCK, NULL, &current) == 0, "sigprocmask query");
    check(sigismember(&current, SIGUSR1), "mask restored");

    printf("success\n");
    return 0;
}
