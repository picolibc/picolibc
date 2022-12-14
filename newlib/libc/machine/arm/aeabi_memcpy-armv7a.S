/*
 * Copyright (c) 2014 ARM Ltd
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

#include "arm-acle-compat.h"

/* NOTE: This ifdef MUST match the one in aeabi_memcpy.c.  */
#if defined (__ARM_ARCH_7A__) && defined (__ARM_FEATURE_UNALIGNED) && \
	(defined (__ARM_NEON__) || !defined (__SOFTFP__))

	.syntax unified
	.global __aeabi_memcpy
	.type   __aeabi_memcpy, %function
__aeabi_memcpy:
	/* Assumes that n >= 0, and dst, src are valid pointers.
          If there is at least 8 bytes to copy, use LDRD/STRD.
          If src and dst are misaligned with different offsets,
          first copy byte by byte until dst is aligned,
          and then copy using LDRD/STRD and shift if needed.
          When less than 8 left, copy a word and then byte by byte.  */

       /* Save registers (r0 holds the return value):
          optimized push {r0, r4, r5, lr}.
          To try and improve performance, stack layout changed,
          i.e., not keeping the stack looking like users expect
          (highest numbered register at highest address).  */
        push {r0, lr}
        strd r4, r5, [sp, #-8]!

        /* Get copying of tiny blocks out of the way first.  */
        /* Is there at least 4 bytes to copy?  */
        subs    r2, r2, #4
        blt     copy_less_than_4       /* If n < 4.  */

        /* Check word alignment.  */
        ands    ip, r0, #3             /* ip = last 2 bits of dst.  */
        bne     dst_not_word_aligned   /* If dst is not word-aligned.  */

        /* Get here if dst is word-aligned.  */
        ands    ip, r1, #3             /* ip = last 2 bits of src.  */
        bne     src_not_word_aligned   /* If src is not word-aligned.  */
word_aligned:
        /* Get here if source and dst both are word-aligned.
           The number of bytes remaining to copy is r2+4.  */

        /* Is there is at least 64 bytes to copy?  */
        subs    r2, r2, #60
        blt     copy_less_than_64                /* If r2 + 4 < 64.  */

        /* First, align the destination buffer to 8-bytes,
           to make sure double loads and stores don't cross cache line boundary,
           as they are then more expensive even if the data is in the cache
           (require two load/store issue cycles instead of one).
           If only one of the buffers is not 8-bytes aligned,
           then it's more important to align dst than src,
           because there is more penalty for stores
           than loads that cross cacheline boundary.
           This check and realignment are only worth doing
           if there is a lot to copy.  */

        /* Get here if dst is word aligned,
           i.e., the 2 least significant bits are 0.
           If dst is not 2w aligned (i.e., the 3rd bit is not set in dst),
           then copy 1 word (4 bytes).  */
        ands    r3, r0, #4
        beq     two_word_aligned  /* If dst already two-word aligned.  */
        ldr     r3, [r1], #4
        str     r3, [r0], #4
        subs    r2, r2, #4
        blt     copy_less_than_64

two_word_aligned:
        /* TODO: Align to cacheline (useful for PLD optimization).  */

        /* Every loop iteration copies 64 bytes.  */
1:
        .irp    offset, #0, #8, #16, #24, #32, #40, #48, #56
        ldrd    r4, r5, [r1, \offset]
        strd    r4, r5, [r0, \offset]
        .endr

        add     r0, r0, #64
        add     r1, r1, #64
        subs    r2, r2, #64
        bge     1b                     /* If there is more to copy.  */

copy_less_than_64:

        /* Get here if less than 64 bytes to copy, -64 <= r2 < 0.
           Restore the count if there is more than 7 bytes to copy.  */
        adds    r2, r2, #56
        blt     copy_less_than_8

        /* Copy 8 bytes at a time.  */
2:
        ldrd    r4, r5, [r1], #8
        strd    r4, r5, [r0], #8
        subs    r2, r2, #8
        bge     2b                     /* If there is more to copy.  */

copy_less_than_8:

        /* Get here if less than 8 bytes to copy, -8 <= r2 < 0.
           Check if there is more to copy.  */
        cmn     r2, #8
        beq     return                          /* If r2 + 8 == 0.  */

        /* Restore the count if there is more than 3 bytes to copy.  */
        adds    r2, r2, #4
        blt     copy_less_than_4

        /* Copy 4 bytes.  */
        ldr     r3, [r1], #4
        str     r3, [r0], #4

copy_less_than_4:
        /* Get here if less than 4 bytes to copy, -4 <= r2 < 0.  */

        /* Restore the count, check if there is more to copy.  */
        adds    r2, r2, #4
        beq     return                          /* If r2 == 0.  */

        /* Get here with r2 is in {1,2,3}={01,10,11}.  */
        /* Logical shift left r2, insert 0s, update flags.  */
        lsls    r2, r2, #31

        /* Copy byte by byte.
           Condition ne means the last bit of r2 is 0.
           Condition cs means the second to last bit of r2 is set,
           i.e., r2 is 1 or 3.  */
        itt     ne
        ldrbne  r3, [r1], #1
        strbne  r3, [r0], #1

        itttt   cs
        ldrbcs  r4, [r1], #1
        ldrbcs  r5, [r1]
        strbcs  r4, [r0], #1
        strbcs  r5, [r0]

return:
        /* Restore registers: optimized pop {r0, r4, r5, pc}   */
        ldrd r4, r5, [sp], #8
        pop {r0, pc}         /* This is the only return point of memcpy.  */

dst_not_word_aligned:

       /* Get here when dst is not aligned and ip has the last 2 bits of dst,
          i.e., ip is the offset of dst from word.
          The number of bytes that remains to copy is r2 + 4,
          i.e., there are at least 4 bytes to copy.
          Write a partial word (0 to 3 bytes), such that dst becomes
	  word-aligned.  */

       /* If dst is at ip bytes offset from a word (with 0 < ip < 4),
          then there are (4 - ip) bytes to fill up to align dst to the next
	  word.  */
        rsb     ip, ip, #4                 /* ip = #4 - ip.  */
        cmp     ip, #2

       /* Copy byte by byte with conditionals.  */
        itt     gt
        ldrbgt  r3, [r1], #1
        strbgt  r3, [r0], #1

        itt     ge
        ldrbge  r4, [r1], #1
        strbge  r4, [r0], #1

        ldrb    lr, [r1], #1
        strb    lr, [r0], #1

       /* Update the count.
          ip holds the number of bytes we have just copied.  */
        subs    r2, r2, ip                        /* r2 = r2 - ip.  */
        blt     copy_less_than_4                  /* If r2 < ip.  */

       /* Get here if there are more than 4 bytes to copy.
          Check if src is aligned.  If beforehand src and dst were not word
	  aligned but congruent (same offset), then now they are both
	  word-aligned, and we can copy the rest efficiently (without
	  shifting).  */
        ands    ip, r1, #3                    /* ip = last 2 bits of src.  */
        beq     word_aligned                  /* If r1 is word-aligned.  */

src_not_word_aligned:
       /* Get here when src is not word-aligned, but dst is word-aligned.
          The number of bytes that remains to copy is r2+4.  */

       /* Copy word by word using LDR when alignment can be done in hardware,
          i.e., SCTLR.A is set, supporting unaligned access in LDR and STR.  */
        subs    r2, r2, #60
        blt     8f

7:
        /* Copy 64 bytes in every loop iteration.  */
        .irp    offset, #0, #4, #8, #12, #16, #20, #24, #28, #32, #36, #40, #44, #48, #52, #56, #60
        ldr     r3, [r1, \offset]
        str     r3, [r0, \offset]
        .endr

        add     r0, r0, #64
        add     r1, r1, #64
        subs    r2, r2, #64
        bge     7b

8:
        /* Get here if less than 64 bytes to copy, -64 <= r2 < 0.
           Check if there is more than 3 bytes to copy.  */
        adds    r2, r2, #60
        blt     copy_less_than_4

9:
       /* Get here if there is less than 64 but at least 4 bytes to copy,
          where the number of bytes to copy is r2+4.  */
        ldr     r3, [r1], #4
        str     r3, [r0], #4
        subs    r2, r2, #4
        bge     9b

        b       copy_less_than_4


	.syntax unified
	.global __aeabi_memcpy4
	.type   __aeabi_memcpy4, %function
__aeabi_memcpy4:
	/* Assumes that both of its arguments are 4-byte aligned.  */

        push {r0, lr}
        strd r4, r5, [sp, #-8]!

        /* Is there at least 4 bytes to copy?  */
        subs    r2, r2, #4
        blt     copy_less_than_4       /* If n < 4.  */

	bl	word_aligned

	.syntax unified
	.global __aeabi_memcpy8
	.type   __aeabi_memcpy8, %function
__aeabi_memcpy8:
	/* Assumes that both of its arguments are 8-byte aligned.  */

        push {r0, lr}
        strd r4, r5, [sp, #-8]!

	/* Is there at least 4 bytes to copy?  */
        subs    r2, r2, #4
        blt     copy_less_than_4	/* If n < 4.  */

        /* Is there at least 8 bytes to copy?  */
        subs    r2, r2, #4
        blt     copy_less_than_8	/* If n < 8.  */

	/* Is there at least 64 bytes to copy?  */
	subs	r2, r2, #56
	blt	copy_less_than_64	/* if n + 8 < 64.  */

	bl	two_word_aligned

#endif
