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
#ifndef _LINUX_RESOURCE_H_
#define _LINUX_RESOURCE_H_
#define LINUX_RLIMIT_CPU      0
#define LINUX_RLIMIT_FSIZE    1
#define LINUX_RLIMIT_DATA     2
#define LINUX_RLIMIT_STACK    3
#define LINUX_RLIMIT_CORE     4
#define LINUX_RLIMIT_NOFILE   7
#define LINUX_RLIMIT_AS       9
#define LINUX_RLIM_INFINITY   4294967295U
#define LINUX_RLIM64_INFINITY 4294967295U
#define LINUX_RLIM_SAVED_MAX  4294967295U
#define LINUX_RLIM_SAVED_CUR  4294967295U

static inline int
_rlimit_to_linux(int resource)
{
    switch (resource) {
    case RLIMIT_CPU:
        return LINUX_RLIMIT_CPU;
    case RLIMIT_FSIZE:
        return LINUX_RLIMIT_FSIZE;
    case RLIMIT_DATA:
        return LINUX_RLIMIT_DATA;
    case RLIMIT_STACK:
        return LINUX_RLIMIT_STACK;
    case RLIMIT_CORE:
        return LINUX_RLIMIT_CORE;
    case RLIMIT_NOFILE:
        return LINUX_RLIMIT_NOFILE;
    case RLIMIT_AS:
        return LINUX_RLIMIT_AS;
    default:
        return -1;
    }
}
#endif /* _LINUX_RESOURCE_H_ */
