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

#include <sys/cdefs.h>
#include <stdint.h>

#if __ARM_ARCH_PROFILE == 'M'

#define STRINGIFY(x) #x

/* Interrupt functions */

void arm_halt_isr(void);

void arm_halt_isr(void)
{
	for(;;);
}

void arm_ignore_isr(void);
void arm_ignore_isr(void)
{
}

#define isr(name) \
	void  arm_ ## name ## _isr(void) __attribute__ ((weak, alias("arm_ignore_isr")));

#define isr_halt(name) \
	void  arm_ ## name ## _isr(void) __attribute__ ((weak, alias("arm_halt_isr")));

isr(nmi);
isr_halt(hardfault);
isr_halt(memmanage);
isr_halt(busfault);
isr_halt(usagefault);
isr(svc);
isr(debugmon);
isr(pendsv);
isr(systick);

void _start(void);
extern uint8_t __stack[];

#define i(addr,name)	[(addr)/4] = (void(*)(void)) arm_ ## name ## _isr

__section(".data.init.enter")
void (* const __weak_interrupt_vector[])(void) __attribute((aligned(128))) = {
	[0] = (void(*)(void))__stack,
	[1] = _start,
	i(0x08, nmi),
	i(0x0c, hardfault),
	i(0x10, memmanage),
	i(0x14, busfault),
	i(0x18, usagefault),
	i(0x2c, svc),
	i(0x30, debugmon),
	i(0x38, pendsv),
	i(0x3c, systick),
};
__weak_reference(__weak_interrupt_vector, __interrupt_vector);

#else

void arm_halt_isr(void);

void __attribute__((naked)) __section(".init")
arm_halt_vector(void)
{
	/* Loop forever. */
	__asm__("1: b 1b");
}

void arm_ignore_isr(void);

void __attribute__((naked)) __section(".init")
arm_ignore_vector(void)
{
	/* Ignore the interrupt by returning */
	__asm__("bx lr");
}

#define vector(name) \
	void  arm_ ## name ## _vector(void) __attribute__ ((weak, alias("arm_ignore_vector")))

#define vector_halt(name) \
	void  arm_ ## name ## _vector(void) __attribute__ ((weak, alias("arm_halt_vector")))

vector_halt(undef);
vector_halt(svc);
vector_halt(prefetch_abort);
vector_halt(data_abort);
vector(not_used);
vector(irq);
vector(fiq);

void __attribute__((naked)) __section(".text.init.enter")
__weak_vector_table(void)
{
	/*
	 * Exception vector that lives at the
	 * start of program text (usually 0x0)
	 */
#if __thumb2__
	/* Thumb 2 processors start in thumb mode */
	__asm__(".thumb");
	__asm__(".syntax unified");
	__asm__("b.w _start");
	__asm__("b.w arm_undef_vector");
	__asm__("b.w arm_svc_vector");
	__asm__("b.w arm_prefetch_abort_vector");
	__asm__("b.w arm_data_abort_vector");
	__asm__("b.w arm_not_used_vector");
	__asm__("b.w arm_irq_vector");
	__asm__("b.w arm_fiq_vector");
#else
	/* Thumb 1 and arm processors start in arm mode */
	__asm__(".arm");
	__asm__("b _start");
	__asm__("b arm_undef_vector");
	__asm__("b arm_svc_vector");
	__asm__("b arm_prefetch_abort_vector");
	__asm__("b arm_data_abort_vector");
	__asm__("b arm_not_used_vector");
	__asm__("b arm_irq_vector");
	__asm__("b arm_fiq_vector");
#endif
}

__weak_reference(__weak_vector_table, __vector_table);

#endif
