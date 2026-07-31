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
#include <unistd.h>
#include <stdio.h>
#include <signal.h>

#define HANDLER_NOT_CALLED 0
#define HANDLER_SUCCESS    1
#define HANDLER_FAILED     2

static volatile sig_atomic_t alarm_result;
static volatile sig_atomic_t alarm_sig;

static void
handle_alarm(int sig)
{
    alarm_sig = sig;
    if (sig == SIGALRM)
        alarm_result = HANDLER_SUCCESS;
    else
        alarm_result = HANDLER_FAILED;
}

int
main(void)
{
    unsigned int prev;
    int          ret = 0;

    alarm_result = HANDLER_NOT_CALLED;

    signal(SIGALRM, handle_alarm);

    setlinebuf(stdout);

    prev = alarm(1);
    if (prev != 0) {
        printf("first alarm call returned non-zero %u\n", prev);
        ret = 1;
    }

    prev = alarm(2);
    if (prev != 1) {
        printf("second alarm call return %u instead of 1\n", prev);
        ret = 1;
    }

    sleep(3);

    switch (alarm_result) {
    case HANDLER_NOT_CALLED:
        printf("signal handler never called\n");
        ret = 1;
        break;
    case HANDLER_FAILED:
        printf("signal handler got signal %d instead of %d\n", alarm_sig, SIGALRM);
        ret = 1;
        break;
    case HANDLER_SUCCESS:
        printf("signal handler called correctly.\n");
        break;
    }

    return ret;
}
