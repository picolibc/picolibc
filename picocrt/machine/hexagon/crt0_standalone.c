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

#include <stdint.h>

#include "../../crt0.h"

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
DECL_HEXAGON_REG(CCR)
DECL_HEXAGON_REG(SYSCFG)

static inline void
isync()
{
    __asm__ __volatile__("isync" : : : "memory");
}

// Copied from hexagon_standalone.h.
static uint32_t
__rdcfg(uint32_t offset)
{
    uint32_t cfgbase;
    __asm__ volatile("%0=cfgbase;" : "=r"(cfgbase));
    __asm__ volatile("%[cfgbase]=asl(%[cfgbase], #5)\n\t"
                     "%[offset]=memw_phys(%[offset], %[cfgbase])"
                     : [cfgbase] "+r"(cfgbase), [offset] "+r"(offset)
                     :
                     :);
    return offset;
}

enum {
    __l2_tag_size = 0x040,
};

static void
enable_l2cache()
{
    unsigned l2cfg;
    unsigned l2tag_sz = __rdcfg(__l2_tag_size);
    switch (l2tag_sz) {
    case 0x80:
        l2cfg = 2;
        break;
    case 0x100:
        l2cfg = 3;
        break;
    case 0x200:
        l2cfg = 4;
        break;
    case 0x400:
        l2cfg = 5;
        break;
    default:
        return;
    }

    // L2 config sequence:
    //    1 - Disable prefetching by clearing HFd/i bits in ssr/ccr
    // Clear HFi, HFd, HFiL2 HFdL2 bits.
    hexagon_write_CCR(hexagon_read_CCR() & ~(0b1111 << 16));

    // Set L2 size to 0 via L2CFG. */
    uint32_t syscfg = hexagon_read_SYSCFG() & ~(0b111 << 16);

    // L2 config sequence:
    //    2 - execute an isync which is aligned to a 32byte boundary.
    __asm__ __volatile__("\
      .p2alignl 5, 0x7f00c000 \n\
      isync");

    // L2 config sequence:
    //    3 - execute an syncht insn to insure there are no outstanding
    //        memory transactions.
    __asm__ __volatile__("syncht");

    // L2 config sequence:
    //    4 - Set the desired L2 size for < V4 (set to 0 for >= V4).
    hexagon_write_SYSCFG(syscfg);
    isync();

    // L2 config sequence:
    //    5 - Execute the L2KILL insn to initiate the cache.
    __asm__ __volatile__("l2kill");
    __asm__ __volatile__("syncht");

    // L2 config sequence:
    //    6 - Set the desired L2 size.
    unsigned l2size = l2cfg;
    if (l2size > 5)
        l2size = 5;
    syscfg = (syscfg & ~0b1111u << 16) | l2size << 16;
    hexagon_write_SYSCFG(syscfg);
    isync();
}

// Configure L1 caches.
static void
enable_l1cache()
{
    hexagon_write_SYSCFG((hexagon_read_SYSCFG() & ~(1 << 2)) | 1 << 1 | 1 << 23);
    isync();
}

void __attribute__((noreturn))
hexagon_start_init1()
{
    // SFD = 0, IE = 0, UM = 0, EX = 0, ASID = 0.
    hexagon_write_SSR(0);
    isync();

    //.InitPcycle:
    hexagon_write_SYSCFG(hexagon_read_SYSCFG() | 1 << 6);
    // Configure IMT/DMT.
    hexagon_write_SYSCFG(hexagon_read_SYSCFG() | 1 << 15);
    //.InitQoS:
    hexagon_write_SYSCFG(hexagon_read_SYSCFG() | 1 << 13);
    hexagon_write_SSR(hexagon_read_SSR() | 1 << 31);

    enable_l2cache();
    enable_l1cache();
    __start();
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
