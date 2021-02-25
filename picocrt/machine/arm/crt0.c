/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2019 Keith Packard
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

#include "../../crt0.h"

#if __ARM_ARCH_PROFILE == 'M'

/*
 * Cortex-mM includes an NVIC and starts with SP initialized, so start
 * is a C function
 */

extern const void *__interrupt_vector[];

#define CPACR	((volatile uint32_t *) (0xE000ED88))

#ifdef __clang__
const void *__interrupt_reference = __interrupt_vector;
#endif

void
_start(void)
{
	/* Generate a reference to __interrupt_vector so we get one loaded */
	__asm__(".equ __my_interrupt_vector, __interrupt_vector");
#ifndef __SOFTFP__
	/* Enable FPU */
	*CPACR |= 0xf << 20;
	/*
	 * Wait for the write enabling FPU to reach memory before
	 * executing the instruction accessing the status register
	 */
	__asm__("dsb");
	__asm__("isb");

	/* Clear FPU status register */
	__asm__("vmsr fpscr, %0" : : "r" (0));
#endif
	__start();
}

#else

/*
 * Regular ARM has an 8-entry exception vector and starts without SP
 * initialized, so start is a naked function
 */

static void __attribute__((used)) __section(".init")
_cstart(void)
{
	__start();
}

extern char __stack[];

void __attribute__((naked)) __section(".init") __attribute__((used))
_start(void)
{
	/* Generate a reference to __vector_table so we get one loaded */
	__asm__(".equ __my_vector_table, __vector_table");

	/* Initialize stack pointer */
	__asm__("mov sp, %0" : : "r" (__stack));

#if __ARM_ARCH == 7
#ifdef __thumb__
	/* Make exceptions run in Thumb mode */
	uint32_t sctlr;
	__asm__("mrc p15, 0, %0, c1, c0, 0" : "=r" (sctlr));
	sctlr |= (1 << 30);
	__asm__("mcr p15, 0, %0, c1, c0, 0" : : "r" (sctlr));
#endif
#ifndef __SOFTFP__
	/* Set CPACR for access to CP10 and 11 */
	__asm__("mcr p15, 0, %0, c1, c0, 2" : : "r" (0xf << 20));
	/* Enable FPU */
	__asm__("vmsr fpexc, %0" : : "r" (0x40000000));
#endif
#endif

	/* Branch to C code */
	__asm__("b _cstart");
}

#endif
