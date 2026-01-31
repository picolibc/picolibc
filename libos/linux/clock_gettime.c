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
#include "local-time.h"

int
clock_gettime(clockid_t id, struct timespec *ts)
{
    struct __kernel_timespec kts;
    int kid;
    int ret;

    if (id == CLOCK_MONOTONIC)
        kid = LINUX_CLOCK_MONOTONIC;
    else if (id == CLOCK_PROCESS_CPUTIME_ID)
        kid = LINUX_CLOCK_PROCESS_CPUTIME_ID;
    else if (id == CLOCK_REALTIME)
        kid = LINUX_CLOCK_REALTIME;
    else if (id == CLOCK_THREAD_CPUTIME_ID)
        kid = LINUX_CLOCK_THREAD_CPUTIME_ID;
    else {
        errno = EINVAL;
        return -1;
    }
    ret = syscall(LINUX_SYS_clock_gettime, kid, &kts);
    if (ret < 0)
        return ret;
    ts->tv_sec = kts.tv_sec;
    ts->tv_nsec = kts.tv_nsec;
    return 0;
}
