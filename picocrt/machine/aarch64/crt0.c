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

/* The size of the thread control block.
 * TLS relocations are generated relative to
 * a location this far *before* the first thread
 * variable (!)
 * NB: The actual size before tp also includes padding
 * to align up to the alignment of .tdata/.tbss.
 */
#if __SIZE_WIDTH__ == 32
extern char __arm32_tls_tcb_offset;
#define TP_OFFSET ((size_t)&__arm32_tls_tcb_offset)
#else
extern char __arm64_tls_tcb_offset;
#define TP_OFFSET ((size_t)&__arm64_tls_tcb_offset)
#endif

#pragma GCC diagnostic ignored "-Warray-bounds"

static inline void
_set_tls(void *tls)
{
	__asm__ volatile("msr tpidr_el0, %0" : : "r" (tls - TP_OFFSET));
}

#include "../../crt0.h"

static void __attribute((used))
_cstart(void)
{
	__start();
}

void __section(".text.init.enter")
_start(void)
{
	/* Initialize stack */
	__asm__("adrp x1, __stack");
	__asm__("add  x1, x1, :lo12:__stack");
	__asm__("mov sp, x1");
	/* Enable FPU */
	__asm__("mov x1, #(0x3 << 20)");
	__asm__("msr cpacr_el1,x1");
	/* Jump into C code */
	__asm__("bl _cstart");
}
