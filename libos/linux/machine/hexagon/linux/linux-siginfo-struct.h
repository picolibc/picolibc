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

#ifndef _LINUX_SIGINFO_STRUCT_H_
#define _LINUX_SIGINFO_STRUCT_H_

struct __kernel_siginfo {
    __int32_t si_signo;
    __int32_t si_errno;
    __int32_t si_code;
    union {
        __int32_t si_pid;
        void     *si_addr;
        __int32_t si_band;
    };
    __uint32_t si_uid;
    union {
        __int32_t             si_status;
        union __kernel_sigval si_value;
    };
    __uint8_t __adjust_24[104];
};

#define SIMPLE_MAP_SIGINFO(_t, _f)         \
    do {                                   \
        (_t)->si_signo = (_f)->si_signo;   \
        (_t)->si_code = (_f)->si_code;     \
        (_t)->si_errno = (_f)->si_errno;   \
        (_t)->si_pid = (_f)->si_pid;       \
        (_t)->si_uid = (_f)->si_uid;       \
        (_t)->si_addr = (_f)->si_addr;     \
        (_t)->si_status = (_f)->si_status; \
        (_t)->si_band = (_f)->si_band;     \
    } while (0)

#endif /* _LINUX_SIGINFO_STRUCT_H_ */
