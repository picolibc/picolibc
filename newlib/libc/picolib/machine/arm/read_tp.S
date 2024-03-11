/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2020 Keith Packard
 * Copyright © 2024 Stephen Street
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
#include "arm_tls.h"
/*
 * This cannot be a C ABI function as the compiler assumes that it
 * does not modify anything other than r0 and lr.
 */
	.syntax unified
	.section .text.__aeabi_read_tp
	.align 4
	.p2align 4,,15
	.global __aeabi_read_tp
	.type __aeabi_read_tp,%function
#ifdef __thumb__
	.thumb
#endif

__aeabi_read_tp:
	.cfi_sections .debug_frame
	.cfi_startproc

#ifdef ARM_TLS_CP15
	mrc p15, 0, r0, c13, c0, 3
	bx lr
#else

#ifdef ARM_RP2040

	/*
	 * Code for RP2040 processor. This is a dual-core cortex-m0
	 * design. We use the CPUID register, stored at offset zero of
	 * the Single-cycle IO block at 0xd0000000
	 */

#define TLS_SIZE	(4 * RP2040_NCORE)

	push {r1,lr}		/* Save R1 (and LR) */
	ldr r1,=0xd0000000	/* Address of SIO->CPUID */
	ldr r1,[r1]	   	/* Fetch active core */
	lsls r1,r1,#2	   	/* Multiply by 4 */
	ldr r0,=__tls		/* Address of __tls array */
	ldr r0,[r0,r1]		/* Fetch __tls[CPUID] */
	pop {r1,pc}		/* Restore R1 and return  */

#else

#define TLS_SIZE	4

	ldr r0,=__tls		/* Load the address of __tls */
	ldr r0,[r0]		/* Dereference to get the value of __tls */
	bx lr			/* All done, return to caller */

#endif

#endif

	.cfi_endproc

#ifndef ARM_TLS_CP15
	.global __tls
	.bss
	.align 4
	.type __tls, %object
	.size __tls, TLS_SIZE
__tls:
	.space TLS_SIZE

#endif
