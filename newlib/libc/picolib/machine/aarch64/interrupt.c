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

#include <stdint.h>

void aarch64_halt_vector(void);

void __section(".init")
aarch64_halt_vector(void)
{
	/* Loop forever. */
        for(;;);
}

void aarch64_ignore_vector(void);

void __section(".init")
aarch64_ignore_vector(void)
{
	/* Ignore the interrupt by returning */
}

#define vector(name) \
	void  aarch64_ ## name ## _vector(void) __attribute__ ((weak, alias("aarch64_ignore_vector")))

#define vector_halt(name) \
	void  aarch64_ ## name ## _vector(void) __attribute__ ((weak, alias("aarch64_halt_vector")))

vector_halt(sync);
vector_halt(serror);
vector(irq);
vector(fiq);

void
__weak_vector_table(void);

void __section(".text.init.enter") __aligned(2048)
__weak_vector_table(void)
{
	/*
	 * Exception vector
	 */
        __asm__("SP_EL0:\n"
                "b aarch64_sync_vector\n"
                ". = SP_EL0 + 0x80\n"
                "b aarch64_irq_vector\n"
                ". = SP_EL0 + 0x100\n"
                "b aarch64_fiq_vector\n"
                ". = SP_EL0 + 0x180\n"
                "b aarch64_serror_vector\n"
                ". = SP_EL0 + 0x200\n"
                "SP_ELx:\n"
                "b aarch64_sync_vector\n"
                ". = SP_ELx + 0x80\n"
                "b aarch64_irq_vector\n"
                ". = SP_ELx + 0x100\n"
                "b aarch64_fiq_vector\n"
                ". = SP_ELx + 0x180\n"
                "b aarch64_serror_vector\n"
                ". = SP_ELx + 0x200\n");
}

__weak_reference(__weak_vector_table, __vector_table);
