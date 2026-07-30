/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the
 * disclaimer below) provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *
 *   * Neither the name of Qualcomm Technologies, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
 * GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
 * HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define _DEFAULT_SOURCE
#include <stdlib.h>
#include <string.h>
#include <sys/lock.h>
#include <limits.h>
#include "local-onexit.h"

#ifdef ENABLE_PICOLIBC_EXIT

struct quick_exit {
    union on_exit_func        func;
    void                     *arg;
    enum pico_quick_exit_kind kind;
};

static struct quick_exit quick_exits[ATEXIT_MAX];

int
_at_quick_exit(enum pico_quick_exit_kind kind, union on_exit_func func, void *arg)
{
    int ret = -1;
    int o;
    __LIBC_LOCK();
    for (o = 0; o < ATEXIT_MAX; o++) {
        if (quick_exits[o].kind == PICO_QUICK_EXIT_EMPTY) {
            quick_exits[o].func = func;
            quick_exits[o].arg = arg;
            quick_exits[o].kind = kind;
            ret = 0;
            break;
        }
    }
    __LIBC_UNLOCK();
    return ret;
}

/*
 * quick_exit runs the functions registered with at_quick_exit and
 * __cxa_at_quick_exit in reverse order of registration, then terminates
 * the program via _Exit. It does not run atexit handlers or flush/close
 * any open streams.
 */
void
quick_exit(int code)
{
    for (;;) {
        int                       i;
        union on_exit_func        func = { 0 };
        enum pico_quick_exit_kind kind = PICO_QUICK_EXIT_EMPTY;
        void                     *arg = 0;

        __LIBC_LOCK();
        for (i = ATEXIT_MAX - 1; i >= 0; i--) {
            kind = quick_exits[i].kind;
            if (kind != PICO_QUICK_EXIT_EMPTY) {
                func = quick_exits[i].func;
                arg = quick_exits[i].arg;
                memset(&quick_exits[i], '\0', sizeof(struct quick_exit));
                break;
            }
        }
        __LIBC_UNLOCK();
        switch (kind) {
        case PICO_QUICK_EXIT_EMPTY:
            _Exit(code);
        case PICO_QUICK_EXIT_ATEXIT:
            func.atexit();
            break;
        case PICO_QUICK_EXIT_CXA_ATEXIT:
            func.cxa_atexit(arg);
            break;
        }
    }
}

#endif
