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

#ifndef _LOCAL_SIGACTION_H_
#define _LOCAL_SIGACTION_H_

#include "local-linux.h"
#include <linux/linux-signal.h>
#include <linux/linux-sigval-struct.h>
#include <linux/linux-siginfo-struct.h>
#include <linux/linux-sigaction-struct.h>

#include <signal.h>

int  _signal_to_linux(int sig);
int  _signal_from_linux(int sig);
void _sigmask_to_linux(__kernel_sigset_t *kset, const sigset_t *set);
void _sigmask_from_linux(sigset_t *set, const __kernel_sigset_t *kset);

#define __KERNEL_NSIG_BYTES (sizeof(__kernel_sigset_t))

static inline void
__kernel_sigset_set_mask(__kernel_sigset_t *ss, int sig)
{
    unsigned si = (sig - 1);
    int      w = si / (sizeof(ss->sa_mask[0]) * 8);
    int      b = si % (sizeof(ss->sa_mask[0]) * 8);

    ss->sa_mask[w] |= (1UL << b);
}

static inline int
__kernel_sigset_get_mask(const __kernel_sigset_t *ss, int sig)
{
    unsigned si = (sig - 1);
    int      w = si / (sizeof(ss->sa_mask[0]) * 8);
    int      b = si % (sizeof(ss->sa_mask[0]) * 8);

    return (ss->sa_mask[w] >> b) & 1;
}

static inline void
__kernel_sa_set_mask(struct __kernel_sigaction *sa, int sig)
{
    __kernel_sigset_set_mask(&sa->sa_mask, sig);
}

static inline int
__kernel_sa_get_mask(struct __kernel_sigaction *sa, int sig)
{
    return __kernel_sigset_get_mask(&sa->sa_mask, sig);
}

#endif /* _LOCAL_SIGACTION_H_ */
