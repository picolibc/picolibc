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

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

static int has[10240];

static bool
check_has(int e)
{
    int i;
    for (i = 0; i < sizeof(has) / sizeof(has[0]); i++) {
        if (has[i] == e)
            return true;
        if (has[i] == 0) {
            has[i] = e;
            return false;
        }
    }
    return true;
}

static void
reset_has(void)
{
    memset(has, 0, sizeof(has));
}

#define PICOLIBC_SIGHUP    1     /* hangup */
#define PICOLIBC_SIGINT    2     /* interrupt */
#define PICOLIBC_SIGQUIT   3     /* quit */
#define PICOLIBC_SIGILL    4     /* illegal instruction (not reset when caught) */
#define PICOLIBC_SIGTRAP   5     /* trace trap (not reset when caught) */
#define PICOLIBC_SIGIOT    6     /* IOT instruction */
#define PICOLIBC_SIGABRT   6     /* used by abort, replace SIGIOT in the future */
#define PICOLIBC_SIGEMT    7     /* EMT instruction */
#define PICOLIBC_SIGFPE    8     /* floating point exception */
#define PICOLIBC_SIGKILL   9     /* kill (cannot be caught or ignored) */
#define PICOLIBC_SIGBUS    10    /* bus error */
#define PICOLIBC_SIGSEGV   11    /* segmentation violation */
#define PICOLIBC_SIGSYS    12    /* bad argument to system call */
#define PICOLIBC_SIGPIPE   13    /* write on a pipe with no one to read it */
#define PICOLIBC_SIGALRM   14    /* alarm clock */
#define PICOLIBC_SIGTERM   15    /* software termination signal from kill */
#define PICOLIBC_SIGURG    16    /* urgent condition on IO channel */
#define PICOLIBC_SIGSTOP   17    /* sendable stop signal not from tty */
#define PICOLIBC_SIGTSTP   18    /* stop signal from tty */
#define PICOLIBC_SIGCONT   19    /* continue a stopped process */
#define PICOLIBC_SIGCHLD   20    /* to parent on child stop or exit */
#define PICOLIBC_SIGCLD    20    /* System V name for SIGCHLD */
#define PICOLIBC_SIGTTIN   21    /* to readers pgrp upon background tty read */
#define PICOLIBC_SIGTTOU   22    /* like TTIN for output if (tp->t_local&LTOSTOP) */
#define PICOLIBC_SIGIO     23    /* input/output possible signal */
#define PICOLIBC_SIGPOLL   PICOLIBC_SIGIO /* System V name for SIGIO */
#define PICOLIBC_SIGXCPU   24    /* exceeded CPU time limit */
#define PICOLIBC_SIGXFSZ   25    /* exceeded file size limit */
#define PICOLIBC_SIGVTALRM 26    /* virtual time alarm */
#define PICOLIBC_SIGPROF   27    /* profiling time alarm */
#define PICOLIBC_SIGWINCH  28    /* window changed */
#define PICOLIBC_SIGLOST   29    /* resource lost (eg, record-lock lost) */
#define PICOLIBC_SIGUSR1   30    /* user defined signal 1 */
#define PICOLIBC_SIGUSR2   31    /* user defined signal 2 */

struct {
    int         linux_value;
    int         picolibc_value;
    const char *name;
} linux_signal[] = {
#define declare_signal(n) { .linux_value = n, .picolibc_value = PICOLIBC_##n, .name = #n }
#ifdef SIGHUP
    declare_signal(SIGHUP),
#endif
#ifdef SIGINT
    declare_signal(SIGINT),
#endif
#ifdef SIGQUIT
    declare_signal(SIGQUIT),
#endif
#ifdef SIGILL
    declare_signal(SIGILL),
#endif
#ifdef SIGTRAP
    declare_signal(SIGTRAP),
#endif
#ifdef SIGIOT
    declare_signal(SIGIOT),
#endif
#ifdef SIGABRT
    declare_signal(SIGABRT),
#endif
#ifdef SIGEMT
    declare_signal(SIGEMT),
#endif
#ifdef SIGFPE
    declare_signal(SIGFPE),
#endif
#ifdef SIGKILL
    declare_signal(SIGKILL),
#endif
#ifdef SIGBUS
    declare_signal(SIGBUS),
#endif
#ifdef SIGSEGV
    declare_signal(SIGSEGV),
#endif
#ifdef SIGSYS
    declare_signal(SIGSYS),
#endif
#ifdef SIGPIPE
    declare_signal(SIGPIPE),
#endif
#ifdef SIGALRM
    declare_signal(SIGALRM),
#endif
#ifdef SIGTERM
    declare_signal(SIGTERM),
#endif
#ifdef SIGURG
    declare_signal(SIGURG),
#endif
#ifdef SIGSTOP
    declare_signal(SIGSTOP),
#endif
#ifdef SIGTSTP
    declare_signal(SIGTSTP),
#endif
#ifdef SIGCONT
    declare_signal(SIGCONT),
#endif
#ifdef SIGCHLD
    declare_signal(SIGCHLD),
#endif
#ifdef SIGCLD
    declare_signal(SIGCLD),
#endif
#ifdef SIGTTIN
    declare_signal(SIGTTIN),
#endif
#ifdef SIGTTOU
    declare_signal(SIGTTOU),
#endif
#ifdef SIGIO
    declare_signal(SIGIO),
#endif
#ifdef SIGPOLL
    declare_signal(SIGPOLL),
#endif
#ifdef SIGXCPU
    declare_signal(SIGXCPU),
#endif
#ifdef SIGXFSZ
    declare_signal(SIGXFSZ),
#endif
#ifdef SIGVTALRM
    declare_signal(SIGVTALRM),
#endif
#ifdef SIGPROF
    declare_signal(SIGPROF),
#endif
#ifdef SIGWINCH
    declare_signal(SIGWINCH),
#endif
#ifdef SIGLOST
    declare_signal(SIGLOST),
#endif
#ifdef SIGUSR1
    declare_signal(SIGUSR1),
#endif
#ifdef SIGUSR2
    declare_signal(SIGUSR2),
#endif
};

#define NLINUX_SIGNAL (sizeof(linux_signal) / sizeof(linux_signal[0]))

int
main(void)
{
    int    max_linux_signal = 0, max_picolibc_signal = 0, s;
    size_t i;

    printf("#define LINUX_SIG_DFL ((_sig_func_ptr) %lld)\n", (long long) (uintptr_t) SIG_DFL);
    printf("#define LINUX_SIG_IGN ((_sig_func_ptr) %lld)\n", (long long) (uintptr_t) SIG_IGN);
    printf("#define LINUX_SIG_ERR ((_sig_func_ptr) %lld)\n", (long long) (uintptr_t) SIG_ERR);

    printf("\n");

    /*
     * Generate an enumeration containing all of the linux signals which
     * are present in the signal list
     */

    printf("enum __linux_signal {\n");
    for (i = 0; i < NLINUX_SIGNAL; i++) {
        printf("    __LINUX_%s = %d,\n",
               linux_signal[i].name, linux_signal[i].linux_value);
        if (linux_signal[i].linux_value > max_linux_signal)
            max_linux_signal = linux_signal[i].linux_value;
    }
    printf("} __attribute__((packed));\n\n");

    printf("enum __picolibc_signal {\n");
    for (i = 0; i < NLINUX_SIGNAL; i++) {
        printf("    __PICOLIBC_%s = %s,\n",
               linux_signal[i].name, linux_signal[i].name);
        if (linux_signal[i].picolibc_value > max_picolibc_signal)
            max_picolibc_signal = linux_signal[i].picolibc_value;
    }
    printf("} __attribute__((packed));\n\n");

    printf("\n");

    reset_has();

    printf("#ifdef SIGNAL_LINUX_TO_PICOLIBC\n");

    printf("static const enum __picolibc_signal __signal_linux_to_picolibc[] = {\n");

    for (s = 0; s <= max_linux_signal; s++) {
        for (i = 0; i < NLINUX_SIGNAL; i++) {
            if (linux_signal[i].linux_value == s)
                break;
        }

        if (i < NLINUX_SIGNAL) {
            if (check_has(linux_signal[i].linux_value))
                continue;
            printf("    [__LINUX_%s] = __PICOLIBC_%s,\n",
                   linux_signal[i].name, linux_signal[i].name);
        } else
            printf("    [%d] = __PICOLIBC_SIGINT,\n", s);
    }

    printf("};\n");
    printf("#endif\n");

    printf("\n");

    reset_has();

    printf("#ifdef SIGNAL_PICOLIBC_TO_LINUX\n");

    printf("static const enum __linux_signal __signal_picolibc_to_linux[] = {\n");

    for (s = 0; s <= max_picolibc_signal; s++) {
        for (i = 0; i < NLINUX_SIGNAL; i++) {
            if (linux_signal[i].picolibc_value == s)
                break;
        }

        if (i < NLINUX_SIGNAL) {
            if (check_has(linux_signal[i].picolibc_value))
                continue;

            printf("    [%s] = __LINUX_%s,\n",
                   linux_signal[i].name, linux_signal[i].name);
        } else
            printf("    [%d] = __LINUX_SIGINT,\n", s);
    }
    printf("};\n");
    printf("#endif\n");

    return 0;
}
