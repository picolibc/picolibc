/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2025 Keith Packard
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

#include <inttypes.h>

struct __attribute__((__packed__)) interrupt_gate {
    uint16_t    offset_low;
    uint16_t    segment;
    uint16_t    flags;
    uint16_t    offset_high;
#ifdef __x86_64
    uint32_t    offset_top;
    uint32_t    reserved;
#endif
};

struct __attribute__((__packed__)) idt {
    uint16_t                    limit;
    const struct interrupt_gate *base;
#if defined(__x86_64) && defined(__ILP32__)
    uint32_t                    pad;
#endif
};

struct interrupt_frame;

#define isr_decl(num, name)                                             \
    void x86_ ## name ## _interrupt(struct interrupt_frame *frame);

isr_decl(0, divide);
isr_decl(2, nmi);
isr_decl(3, breakpoint);
isr_decl(6, invalid_opcode);
isr_decl(11, segment_not_present);
isr_decl(12, stack_segment_fault);
isr_decl(13, general_protection_fault);
isr_decl(14, page_fault);
isr_decl(18, machine_check);

#define GATE_TYPE_TASK          5
#define GATE_TYPE_ISR_286       6
#define GATE_TYPE_TRAP_286      7
#define GATE_TYPE_ISR_386       14
#define GATE_TYPE_TRAP_386      15

#define GATE_SEGMENT_PRESENT    (1 << 15)
#define GATE_DPL(l)             ((l) << 13)
#define GATE_TYPE(t)            ((t) << 8)
#define GATE_OFFSET_LOW(p)      ((

#define GATE_FLAGS(t)   (GATE_SEGMENT_PRESENT | GATE_DPL(3) | GATE_TYPE(t))
#define GATE_ISR(n)     ((uintptr_t) x86_ ## n ## _interrupt)
#define GATE_TRAP(n)    ((uintptr_t) x86_ ## n ## _interrupt)

#define isr(number, name) [number] = GATE_ISR(name)
#define esr(number, name) [number] = GATE_TRAP(name)

/*
 * We can't construct the interrupt vector at compile time
 * because the linker can't perform the necessary relocations.
 * Kludge around this by building a constructor to initialize
 * them at runtime.
 */
static struct interrupt_gate interrupt_vector[256];

const uintptr_t __weak_interrupt_table[256] = {
    esr(0, divide),
    esr(2, nmi),
    esr(3, breakpoint),
    esr(6, invalid_opcode),
    esr(11, segment_not_present),
    esr(12, stack_segment_fault),
    esr(13, general_protection_fault),
    esr(14, page_fault),
    esr(18, machine_check),
};

__weak_reference(__weak_interrupt_table, __interrupt_table);

static void __attribute__((constructor))
init_interrupt_vector(void)
{
    unsigned i;
    for (i = 0; i < 256; i++) {
        uintptr_t       ptr = __interrupt_table[i];
        interrupt_vector[i].offset_low = (uint16_t) ptr;
        interrupt_vector[i].segment = 0x10;
        if (i < 32)
            interrupt_vector[i].flags = GATE_FLAGS(GATE_TYPE_TRAP_386);
        else
            interrupt_vector[i].flags = GATE_FLAGS(GATE_TYPE_ISR_386);
        interrupt_vector[i].offset_high = (uint16_t) (ptr >> 16);
#ifdef __x86_64
        interrupt_vector[i].offset_top = (uint32_t) ((uint64_t) ptr >> 32);
#endif
    }
}

/* Interrupt descriptor table value for lidt */
struct idt __weak_idt = {
    .limit = sizeof(interrupt_vector) - 1,
    .base = interrupt_vector
};

__weak_reference(__weak_idt, __idt);
