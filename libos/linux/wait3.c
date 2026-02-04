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

#include "local-linux.h"
#include <sys/wait.h>

pid_t
wait3(int *wstatus, int options, struct rusage *rusage)
{
    int   koptions = 0;
    pid_t ret;
    int   kstatus;
    struct {
        struct rusage usage;
        long          space[16];
    } kusage, *pusage = NULL;

    if (options & WNOHANG)
        koptions |= LINUX_WNOHANG;
    if (options & WUNTRACED)
        koptions |= LINUX_WUNTRACED;
    if (options & WCONTINUED)
        koptions |= LINUX_WCONTINUED;
    if (rusage)
        pusage = &kusage;
    ret = syscall(LINUX_SYS_wait4, -1, &kstatus, koptions, pusage);
    if (ret != (pid_t)-1) {
        if (WIFSIGNALED(kstatus))
            kstatus = __W_EXITCODE(0, _signal_from_linux(WTERMSIG(kstatus)));
        *wstatus = kstatus;
        if (rusage)
            *rusage = kusage.usage;
    }
    return ret;
}
