/*
Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
SPDX-License-Identifier: BSD-3-Clause-Clear

Redistribution and use in source and binary forms, with or without
modification, are permitted (subject to the limitations in the
disclaimer below) provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following
    disclaimer in the documentation and/or other materials provided
    with the distribution.

  * Neither the name of Qualcomm Technologies, Inc. nor the names of its
    contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#define CRT0_GET_CMDLINE

#include "../../crt0.h"

#ifdef CRT0_SEMIHOST

#define DECL_HEXAGON_REG(R)                          \
    static inline uint32_t hexagon_read_##R()        \
    {                                                \
        uint32_t x;                                  \
        __asm __volatile("%0=" #R : "=r"(x));        \
        return x;                                    \
    }                                                \
    static inline void hexagon_write_##R(uint32_t x) \
    {                                                \
        __asm __volatile(#R "=%0" : : "r"(x));       \
    }

// Monitor mode registers.
DECL_HEXAGON_REG(SSR)

static inline void
isync()
{
    __asm__ __volatile__("isync" : : : "memory");
}

void __attribute__((noreturn))
coredump()
{
    hexagon_write_SSR(hexagon_read_SSR() & ~(1 << 16 | 1 << 17)); // UM = 0, EX = 0.
    isync();
    register uint32_t r0 __asm__("r0") = 0xcd;
    __asm__ __volatile__("trap0(#0)" : "=r"(r0) : "r"(r0));
    __asm__ __volatile__("stop(%0)" : : "r"(-1));
    __builtin_unreachable();
}
#endif

void __attribute__((noreturn))
_cstart()
{
#ifdef CRT0_SEMIHOST
    hexagon_write_SSR(0);
#endif
    __start();
}
