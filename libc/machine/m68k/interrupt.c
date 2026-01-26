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

static void
m68k_halt_isr(void)
{
    for (;;)
        ;
}

static void
m68k_ignore_isr(void)
{
}

void        _start(void);

extern char __stack[];

#define isr(name)      void m68k_##name##_isr(void) __attribute__((weak, alias("m68k_ignore_isr")));

#define isr_halt(name) void m68k_##name##_isr(void) __attribute__((weak, alias("m68k_halt_isr")));

isr_halt(access_fault);
isr_halt(address_error);
isr_halt(illegal_instruction);
isr_halt(divide_by_zero);
isr(chk);
isr(trap);

isr_halt(fp_branch_uc);
isr_halt(fp_inexact);
isr_halt(fp_divide_by_zero);
isr_halt(fp_underflow);

isr_halt(fp_operand_error);
isr_halt(fp_overflow);
isr_halt(fp_signaling_nan);
isr_halt(fp_unimplemented_data);

#define i(id, name) [id] = m68k_##name##_isr

__section(".data.init.enter") void * const __exception_vector[] = {
    [0] = __stack,
    [1] = _start,
    i(2, access_fault),
    i(3, address_error),

    i(4, illegal_instruction),
    i(5, divide_by_zero),
    i(6, chk),
    i(7, trap),

    i(48, fp_branch_uc),
    i(49, fp_inexact),
    i(50, fp_divide_by_zero),
    i(51, fp_underflow),

    i(52, fp_operand_error),
    i(53, fp_overflow),
    i(54, fp_signaling_nan),
    i(55, fp_unimplemented_data),
};
