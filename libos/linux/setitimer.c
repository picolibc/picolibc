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

#include "local-time.h"

int
setitimer(int which, const struct itimerval *new_value, struct itimerval *old_value)
{
    struct __kernel_itimerval kit, okit = {}, *pokit = NULL;
    int                       ret;

    switch (which) {
    case ITIMER_REAL:
        which = LINUX_ITIMER_REAL;
        break;
    case ITIMER_VIRTUAL:
        which = LINUX_ITIMER_VIRTUAL;
        break;
    case ITIMER_PROF:
        which = LINUX_ITIMER_PROF;
        break;
    default:
        errno = EINVAL;
        return -1;
    }

    MAP_ITV(&kit, new_value);

#if __SIZEOF_LONG__ <= __SIZEOF_INT__
    /*
     * The kernel's itimerval holds seconds in a long, and Linux has no
     * 64-bit version of setitimer. As alarm does, clamp to the range it
     * holds, about 68 years each way, rather than wrapping
     */
    if (new_value->it_value.tv_sec > __LONG_MAX__)
        kit.it_value.tv_sec = __LONG_MAX__;
    else if (new_value->it_value.tv_sec < -__LONG_MAX__ - 1)
        kit.it_value.tv_sec = -__LONG_MAX__ - 1;
    if (new_value->it_interval.tv_sec > __LONG_MAX__)
        kit.it_interval.tv_sec = __LONG_MAX__;
    else if (new_value->it_interval.tv_sec < -__LONG_MAX__ - 1)
        kit.it_interval.tv_sec = -__LONG_MAX__ - 1;
#endif

    if (old_value)
        pokit = &okit;

    ret = _syscall(LINUX_SYS_setitimer, which, &kit, pokit);

    if (old_value)
        MAP_ITV(old_value, &okit);
    return ret;
}
