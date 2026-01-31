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

#define _GNU_SOURCE
#include <time.h>
#include <stdio.h>

int
main(void)
{
#ifdef CLOCK_MONOTONIC
    printf("#define %-32.32s 0x%08x\n", "LINUX_CLOCK_MONOTONIC", CLOCK_MONOTONIC);
#endif
#ifdef CLOCK_PROCESS_CPUTIME_ID
    printf("#define %-32.32s 0x%08x\n", "LINUX_CLOCK_PROCESS_CPUTIME_ID", CLOCK_PROCESS_CPUTIME_ID);
#endif
#ifdef CLOCK_REALTIME
    printf("#define %-32.32s 0x%08x\n", "LINUX_CLOCK_REALTIME", CLOCK_REALTIME);
#endif
#ifdef CLOCK_THREAD_CPUTIME_ID
    printf("#define %-32.32s 0x%08x\n", "LINUX_CLOCK_THREAD_CPUTIME_ID", CLOCK_THREAD_CPUTIME_ID);
#endif
#ifdef TIMER_ABSTIME
    printf("#define %-32.32s 0x%08x\n", "LINUX_TIMER_ABSTIME", TIMER_ABSTIME);
#endif
    return 0;
}
