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
#include <signal.h>
#include "local-sigaction.h"

int
raise(int sig)
{
    /*
     * raise() must deliver the signal to the calling thread.
     *
     * kill(getpid(), sig) is process-directed: the kernel may pick any
     * thread in the process to run the handler, and it may leave the
     * signal merely pending rather than delivering it synchronously.
     * That is visible even in single-threaded programs when a signal is
     * raised again from inside its own handler -- the recursive delivery
     * can be lost and the interrupted context resumed incorrectly.
     *
     * Prefer tgkill(), which names both the thread group and the thread
     * and so cannot be confused by a recycled thread id.  tkill() is an
     * obsolete predecessor to tgkill() and is only used when tgkill() is
     * unavailable.
     */
#if defined(LINUX_SYS_tgkill) && defined(LINUX_SYS_getpid) && defined(LINUX_SYS_gettid)
    int pid = syscall(LINUX_SYS_getpid);
    int tid;

    if (pid < 0)
        return pid;

    tid = syscall(LINUX_SYS_gettid);
    if (tid < 0)
        return tid;

    return syscall(LINUX_SYS_tgkill, pid, tid, _signal_to_linux(sig));
#elif defined(LINUX_SYS_tkill) && defined(LINUX_SYS_gettid)
    int tid = syscall(LINUX_SYS_gettid);

    if (tid < 0)
        return tid;

    return syscall(LINUX_SYS_tkill, tid, _signal_to_linux(sig));
#else
    return kill(getpid(), sig);
#endif
}
