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

#ifndef _LOCAL_TIME_H_
#define _LOCAL_TIME_H_

#include "local-linux.h"
#include <linux/linux-timeval-struct.h>
#include <linux/linux-timespec-struct.h>
#include <linux/linux-itimerspec-struct.h>
#include <linux/linux-itimerval-struct.h>

#include <time.h>

/*
 * Reading the clock.
 *
 * Architectures with a 64-bit time_t ABI provide clock_gettime, which
 * takes a struct __kernel_timespec with a 64-bit tv_sec.
 *
 * 32-bit architectures added after the y2038 work (riscv32, hexagon,
 * ...) do not provide clock_gettime at all; the only entry point is
 * clock_gettime64. Older 32-bit architectures provide both: their
 * clock_gettime is the y2038-limited *_time32 kernel handler, while
 * clock_gettime64 is y2038-safe. Prefer clock_gettime64 whenever the
 * kernel offers it.
 *
 * The two syscalls do not share a layout: clock_gettime64 always
 * expects a 64-bit tv_sec/tv_nsec pair, whereas the generated struct
 * __kernel_timespec follows the arch's native time_t and is 32-bit on
 * those targets. Pair each syscall with a matching struct so the kernel
 * never writes past the end of the object.
 */
#ifdef LINUX_SYS_clock_gettime64

#define LINUX_SYS_clock_gettime_time_t LINUX_SYS_clock_gettime64

struct __kernel_timespec_time_t {
    __int64_t tv_sec;
    __int64_t tv_nsec;
};

#else

#define LINUX_SYS_clock_gettime_time_t LINUX_SYS_clock_gettime

#define __kernel_timespec_time_t       __kernel_timespec

#endif

/* Copy between kernel and library representations of itimerval */
#define MAP_ITV(a, b)                                             \
    do {                                                          \
        SIMPLE_MAP_TIMEVAL(&(a)->it_interval, &(b)->it_interval); \
        SIMPLE_MAP_TIMEVAL(&(a)->it_value, &(b)->it_value);       \
    } while (0)

#endif /* _LOCAL_TIME_H_ */
