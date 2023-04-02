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

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define HANDLER_NOT_CALLED      0
#define HANDLER_SUCCESS         1
#define HANDLER_FAILED          2

static sig_atomic_t     handler_result;
static sig_atomic_t     handler_sig_expect;
static sig_atomic_t     handler_sig_received;

static void
signal_handler(int sig)
{
    handler_sig_received = sig;
    if (sig == (int) handler_sig_expect)
        handler_result = HANDLER_SUCCESS;
    else
        handler_result = HANDLER_FAILED;
}

static const int test_signals[] = {
    SIGABRT,
    SIGFPE,
    SIGILL,
    SIGINT,
    SIGSEGV,
    SIGTERM,
};

#define MODE_IGN        0
#define MODE_CATCH      1
#define MODE_MAX        2

#define NTEST_SIG    (sizeof(test_signals)/sizeof(test_signals[0]))

int main(void)
{
    void (*old_func)(int);
    void (*new_func)(int);
    void (*prev_func)(int);
    int ret;
    unsigned u;
    int fail = 0;
    int mode;

    for (u = 0; u < NTEST_SIG; u++) {
        int sig = test_signals[u];
        prev_func = SIG_DFL;
        for (mode = 0; mode < MODE_MAX; mode++) {
            switch (mode) {
            case MODE_IGN:
                new_func = SIG_IGN;
                break;
            case MODE_CATCH:
                new_func = signal_handler;
                break;
            }

            /* Set up the signal handler */
            old_func = signal(sig, new_func);
            if (old_func != SIG_DFL) {
                printf("signal %d: old_func was %p, not SIG_DFL\n",
                       sig, old_func);
                fail = 1;
            }

            /* Invoke the handler */
            prev_func = new_func;
            handler_result = HANDLER_NOT_CALLED;
            handler_sig_expect = (sig_atomic_t) sig;

            ret = raise(sig);
            if (ret != 0) {
                printf("signal %d: raise returned %d\n", sig, ret);
                fail = 1;
            }

            /* Validate the results */
            switch (mode) {
            case MODE_IGN:
                if (handler_result != HANDLER_NOT_CALLED) {
                    printf("signal %d: handler called with SIG_IGN\n", sig);
                    fail = 1;
                }
                break;
            case MODE_CATCH:
                switch (handler_result) {
                case HANDLER_NOT_CALLED:
                    printf("signal %d: handler not called\n", sig);
                    fail = 1;
                    break;
                case HANDLER_SUCCESS:
                    prev_func = SIG_DFL;
                    break;
                case HANDLER_FAILED:
                    printf("signal %d: handler failed, received signal %d\n",
                           sig, (int) handler_sig_received);
                    fail = 1;
                    break;
                }
                break;
            default:
                fail = 1;
                break;
            }

            old_func = signal(sig, SIG_DFL);
            if (old_func != prev_func) {
                printf("signal %d: after test, signal is %p expected %p\n",
                       sig, old_func, prev_func);
                fail = 1;
            }
        }
    }
    exit(fail);
}
