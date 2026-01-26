/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2024 Keith Packard
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

#include <stdint.h>

/* Interrupt functions */

void arc_halt_vector(void);

void
arc_halt_vector(void)
{
    for (;;)
        ;
}

void arc_ignore_vector(void);

void
arc_ignore_vector(void)
{
}

#define isr(name) void arc_##name##_vector(void) __attribute__((weak, alias("arc_ignore_vector")));

#define isr_halt(name)                                                              \
    void arc_##name##_vector(void) __attribute__((weak, alias("arc_halt_vector")));

isr_halt(memory_error);
isr_halt(instruction_error);
isr_halt(machine_check);
isr(tlb_miss_i);
isr(tlb_miss_d);
isr(prot_v);
isr(privilege_v);
isr(swi);
isr(trap);
isr(extension);
isr(div_zero);
/* dc_error is unused in ARCv3 and de-facto unused in ARCv2 as well */
isr(dc_error);
isr(maligned);

void           _start(void);
extern uint8_t __stack[];

#define i(addr, name) [(addr) / 4] = (void (*)(void))arc_##name##_vector

__section(".data.init.enter") void (* const __weak_vector_table[])(void) __attribute((aligned(128)))
= {
      [0] = (void *)_start,   i(0x04, memory_error), i(0x08, instruction_error),
      i(0x0c, machine_check), i(0x10, tlb_miss_i),   i(0x14, tlb_miss_d),
      i(0x18, prot_v),        i(0x1c, privilege_v),  i(0x20, swi),
      i(0x24, trap),          i(0x28, extension),    i(0x2c, div_zero),
      i(0x30, dc_error),      i(0x34, maligned),
  };
__weak_reference(__weak_vector_table, __vector_table);
