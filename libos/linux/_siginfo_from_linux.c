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

#include "local-sigaction.h"

/*
 * Convert the kernel's siginfo to picolibc's, translating the signal
 * number and the si_code values picolibc defines. Codes specific to a
 * signal have no picolibc values, so they become 0.
 */
void
_siginfo_from_linux(siginfo_t *info, const struct __kernel_siginfo *kinfo)
{
    SIMPLE_MAP_SIGINFO(info, kinfo);
    SIMPLE_MAP_SIGVAL(&info->si_value, &kinfo->si_value);
    info->si_signo = _signal_from_linux(kinfo->si_signo);
    switch (kinfo->si_code) {
    case LINUX_SI_USER:
    case LINUX_SI_TKILL:
        info->si_code = SI_USER;
        break;
    case LINUX_SI_QUEUE:
        info->si_code = SI_QUEUE;
        break;
    case LINUX_SI_TIMER:
        info->si_code = SI_TIMER;
        break;
    case LINUX_SI_MESGQ:
        info->si_code = SI_MESGQ;
        break;
    case LINUX_SI_ASYNCIO:
        info->si_code = SI_ASYNCIO;
        break;
    default:
        info->si_code = 0;
        break;
    }
}
