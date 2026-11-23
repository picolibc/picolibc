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
#include <signal.h>
#include <stdio.h>
#include <string.h>

/*
 * strsignal: a message for every signal <signal.h> defines, distinct
 * messages for distinct signals, and a string holding the number, never
 * a null pointer, for numbers that aren't signals; and psignal, which
 * mustn't overwrite strsignal's string
 */

#define S(name) { name, #name }

static const struct {
    int         value;
    const char *name;
} signals[] = {
    S(SIGABRT),   S(SIGFPE), S(SIGILL), S(SIGINT), S(SIGSEGV), S(SIGTERM),
#ifdef SIGALRM
    S(SIGALRM),
#endif
#ifdef SIGBUS
    S(SIGBUS),
#endif
#ifdef SIGCHLD
    S(SIGCHLD),
#endif
#ifdef SIGCONT
    S(SIGCONT),
#endif
#ifdef SIGHUP
    S(SIGHUP),
#endif
#ifdef SIGKILL
    S(SIGKILL),
#endif
#ifdef SIGPIPE
    S(SIGPIPE),
#endif
#ifdef SIGQUIT
    S(SIGQUIT),
#endif
#ifdef SIGSTOP
    S(SIGSTOP),
#endif
#ifdef SIGTSTP
    S(SIGTSTP),
#endif
#ifdef SIGTTIN
    S(SIGTTIN),
#endif
#ifdef SIGTTOU
    S(SIGTTOU),
#endif
#ifdef SIGUSR1
    S(SIGUSR1),
#endif
#ifdef SIGUSR2
    S(SIGUSR2),
#endif
#ifdef SIGWINCH
    S(SIGWINCH),
#endif
#ifdef SIGPROF
    S(SIGPROF),
#endif
#ifdef SIGSYS
    S(SIGSYS),
#endif
#ifdef SIGTRAP
    S(SIGTRAP),
#endif
#ifdef SIGURG
    S(SIGURG),
#endif
#ifdef SIGVTALRM
    S(SIGVTALRM),
#endif
#ifdef SIGXCPU
    S(SIGXCPU),
#endif
#ifdef SIGXFSZ
    S(SIGXFSZ),
#endif
};

#define N_SIGNALS (sizeof(signals) / sizeof(signals[0]))

#define MSG_MAX   256

static char messages[N_SIGNALS][MSG_MAX];

int
main(void)
{
    const char *p;
    size_t      i, j;
    int         sig, ret = 0;

    for (i = 0; i < N_SIGNALS; i++) {
        p = strsignal(signals[i].value);
        if (p == NULL || *p == '\0') {
            printf("strsignal(%s) gave no message\n", signals[i].name);
            ret = 1;
            continue;
        }
        snprintf(messages[i], sizeof(messages[i]), "%s", p);
    }

    /* Distinct signals have distinct messages */
    for (i = 0; i < N_SIGNALS; i++)
        for (j = i + 1; j < N_SIGNALS; j++)
            if (signals[i].value != signals[j].value && messages[i][0]
                && strcmp(messages[i], messages[j]) == 0) {
                printf("%s and %s share the message \"%s\"\n", signals[i].name, signals[j].name,
                       messages[i]);
                ret = 1;
            }

    /* Every other number gives a string too */
    for (sig = -2; sig <= 70; sig++) {
        p = strsignal(sig);
        if (p == NULL || *p == '\0') {
            printf("strsignal(%d) gave no message\n", sig);
            ret = 1;
        }
    }

    /*
     * No function calls strsignal: psignal leaves the string strsignal
     * returned alone, even one built from a number
     */
    {
        static const int pairs[][2] = {
            { 70, 71 },
            { 70, -6 },
            { 70, 2  },
        };

        for (i = 0; i < sizeof(pairs) / sizeof(pairs[0]); i++) {
            char copy[MSG_MAX];

            p = strsignal(pairs[i][0]);
            snprintf(copy, sizeof(copy), "%s", p);
            psignal(pairs[i][1], "psignal, which mustn't call strsignal");
            if (strcmp(p, copy) != 0) {
                printf("psignal(%d) changed strsignal(%d)'s string from \"%s\" to \"%s\"\n",
                       pairs[i][1], pairs[i][0], copy, p);
                ret = 1;
            }
        }
    }

    if (ret == 0)
        printf("success\n");
    return ret;
}
