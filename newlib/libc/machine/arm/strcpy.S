/*
 * Copyright (c) 2008 ARM Ltd
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the company may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ARM LTD ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ARM LTD BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "arm_asm.h"

#ifdef __thumb2__
#define magic1(REG) #0x01010101
#define magic2(REG) #0x80808080
#else
#define magic1(REG) REG
#define magic2(REG) REG , lsl #7
#endif

        .syntax unified
#ifdef __thumb__
	.thumb
#endif
	.text
	.global strcpy
	.type strcpy, %function
strcpy:
	.fnstart
	.cfi_sections .debug_frame
	.cfi_startproc
	prologue push_ip=HAVE_PAC_LEAF
	
#if !(defined(__OPTIMIZE_SIZE__) || defined (PREFER_SIZE_OVER_SPEED) || \
      (defined (__thumb__) && !defined (__thumb2__)))
	
#ifdef _ISA_ARM_7
        pld	[r1]
#endif
        eor	r2, r0, r1
        mov	ip, r0
        tst	r2, #3
        bne	4f
        tst	r1, #3
        bne	3f
5:
#ifndef __thumb2__
        str	r5, [sp, #-4]!
        mov	r5, #0x01
        orr	r5, r5, r5, lsl #8
        orr	r5, r5, r5, lsl #16
#endif

        str	r4, [sp, #-4]!
        tst	r1, #4
        ldr	r3, [r1], #4
        beq	2f
        sub	r2, r3, magic1(r5)
        bics	r2, r2, r3
        tst	r2, magic2(r5)
        itt	eq
        streq	r3, [ip], #4
        ldreq	r3, [r1], #4
        bne	1f
       /* Inner loop.  We now know that r1 is 64-bit aligned, so we
	  can safely fetch up to two words.  This allows us to avoid
	  load stalls.  */
        .p2align 2
2:
#ifdef _ISA_ARM_7
        pld	[r1, #8]
#endif
        ldr	r4, [r1], #4
        sub	r2, r3, magic1(r5)
        bics	r2, r2, r3
        tst	r2, magic2(r5)
        sub	r2, r4, magic1(r5)
        bne	1f
        str	r3, [ip], #4
        bics	r2, r2, r4
        tst	r2, magic2(r5)
        itt	eq
        ldreq	r3, [r1], #4
        streq	r4, [ip], #4
        beq	2b
        mov	r3, r4
1:
#ifdef __ARMEB__
        rors	r3, r3, #24
#endif
        strb	r3, [ip], #1
        tst	r3, #0xff
#ifdef __ARMEL__
        ror	r3, r3, #8
#endif
        bne	1b
        ldr	r4, [sp], #4
#ifndef __thumb2__
        ldr	r5, [sp], #4
#endif
        bx	lr

       /* Strings have the same offset from word alignment, but it's
	  not zero.  */
3:
        tst	r1, #1
        beq	1f
        ldrb	r2, [r1], #1
        strb	r2, [ip], #1
        cmp	r2, #0
        it	eq
        bxeq	lr
1:
        tst	r1, #2
        beq	5b
        ldrh	r2, [r1], #2
#ifdef __ARMEB__
        tst	r2, #0xff00
        iteet	ne
        strhne	r2, [ip], #2
        lsreq	r2, r2, #8
        strbeq	r2, [ip]
        tstne	r2, #0xff
#else
        tst	r2, #0xff
        itet	ne
        strhne	r2, [ip], #2
        strbeq	r2, [ip]
        tstne	r2, #0xff00
#endif
        bne	5b
	.cfi_remember_state
	epilogue push_ip=HAVE_PAC_LEAF
	.cfi_restore_state

       /* src and dst do not have a common word-alignement.  Fall back to
	  byte copying.  */
4:
        ldrb	r2, [r1], #1
        strb	r2, [ip], #1
        cmp	r2, #0
        bne	4b

#elif !defined (__thumb__) || defined (__thumb2__)
        mov	r3, r0
        1:
        ldrb	r2, [r1], #1
        strb	r2, [r3], #1
        cmp	r2, #0
        bne	1b
#else
        movs	r3, r0
1:
        ldrb	r2, [r1]
        adds	r1, #1
        strb	r2, [r3]
        adds	r3, #1
        cmp	r2, #0
        bne	1b
#endif
	epilogue push_ip=HAVE_PAC_LEAF
	.cfi_endproc
	.cantunwind
	.fnend
	.size strcpy, . - strcpy

#if defined(__linux__) && defined(__ELF__)
.section .note.GNU-stack,"",%progbits
#endif
