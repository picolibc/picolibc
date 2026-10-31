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
clock_nanosleep(clockid_t id, int flags, const struct timespec *dur, struct timespec *rem)
{
    struct __kernel_timespec_time_t kdur;
    struct __kernel_timespec_time_t krem, *pkrem = NULL;
    int                             kid;
    int                             kflags = 0;
    int                             ret;

    MAP_CLOCK_ID(kid, id);
    if (flags == TIMER_ABSTIME)
        kflags = LINUX_TIMER_ABSTIME;
    kdur.tv_sec = dur->tv_sec;
    kdur.tv_nsec = dur->tv_nsec;
    if (rem)
        pkrem = &krem;
    ret = syscall(LINUX_SYS_clock_nanosleep_time_t, kid, kflags, &kdur, pkrem);
    if (ret < 0)
        return ret;
    if (rem) {
        rem->tv_sec = krem.tv_sec;
        rem->tv_nsec = krem.tv_nsec;
    }
    return 0;
}
