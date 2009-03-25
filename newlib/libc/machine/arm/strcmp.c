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
#include <_ansi.h>
#include <string.h>

#ifdef __ARMEB__
#define SHFT2LSB "lsl"
#define SHFT2MSB "lsr"
#define MSB "0x000000ff"
#define LSB "0xff000000"
#else
#define SHFT2LSB "lsr"
#define SHFT2MSB "lsl"
#define MSB "0xff000000"
#define LSB "0x000000ff"
#endif

#ifdef __thumb2__
#define magic1(REG) "#0x01010101"
#define magic2(REG) "#0x80808080"
#else
#define magic1(REG) #REG
#define magic2(REG) #REG ", lsl #7"
#endif

int 
__attribute__((naked)) strcmp (const char* s1, const char* s2)
{
  asm(
#if !(defined(__OPTIMIZE_SIZE__) || defined (PREFER_SIZE_OVER_SPEED) || \
      (defined (__thumb__) && !defined (__thumb2__)))
      "optpld	r0\n\t"
      "optpld	r1\n\t"
      "eor	r2, r0, r1\n\t"
      "tst	r2, #3\n\t"
      /* Strings not at same byte offset from a word boundary.  */
      "bne	strcmp_unaligned\n\t"
      "ands	r2, r0, #3\n\t"
      "bic	r0, r0, #3\n\t"
      "bic	r1, r1, #3\n\t"
      "ldr	ip, [r0], #4\n\t"
      "it	eq\n\t"
      "ldreq	r3, [r1], #4\n\t"
      "beq	1f\n\t"
      /* Although s1 and s2 have identical initial alignment, they are
	 not currently word aligned.  Rather than comparing bytes,
	 make sure that any bytes fetched from before the addressed
	 bytes are forced to 0xff.  Then they will always compare
	 equal.  */
      "eor	r2, r2, #3\n\t"
      "lsl	r2, r2, #3\n\t"
      "mvn	r3, #"MSB"\n\t"
      SHFT2LSB"	r2, r3, r2\n\t"
      "ldr	r3, [r1], #4\n\t"
      "orr	ip, ip, r2\n\t"
      "orr	r3, r3, r2\n"
 "1:\n\t"
#ifndef __thumb2__
      /* Load the 'magic' constant 0x01010101.  */
      "str	r4, [sp, #-4]!\n\t"
      "mov	r4, #1\n\t"
      "orr	r4, r4, r4, lsl #8\n\t"
      "orr	r4, r4, r4, lsl #16\n"
#endif
      ".p2align	2\n"
 "4:\n\t"
      "optpld	r0, #8\n\t"
      "optpld	r1, #8\n\t"
      "sub	r2, ip, "magic1(r4)"\n\t"
      "cmp	ip, r3\n\t"
      "itttt	eq\n\t"
      /* check for any zero bytes in first word */
      "biceq	r2, r2, ip\n\t"
      "tsteq	r2, "magic2(r4)"\n\t"
      "ldreq	ip, [r0], #4\n\t"
      "ldreq	r3, [r1], #4\n\t"
      "beq	4b\n"
 "2:\n\t"
      /* There's a zero or a different byte in the word */
      SHFT2MSB"	r0, ip, #24\n\t"
      SHFT2LSB"	ip, ip, #8\n\t"
      "cmp	r0, #1\n\t"
      "it	cs\n\t"
      "cmpcs	r0, r3, "SHFT2MSB" #24\n\t"
      "it	eq\n\t"
      SHFT2LSB"eq r3, r3, #8\n\t"
      "beq	2b\n\t"
      /* On a big-endian machine, r0 contains the desired byte in bits
	 0-7; on a little-endian machine they are in bits 24-31.  In
	 both cases the other bits in r0 are all zero.  For r3 the
	 interesting byte is at the other end of the word, but the
	 other bits are not necessarily zero.  We need a signed result
	 representing the differnece in the unsigned bytes, so for the
	 little-endian case we can't just shift the interesting bits
	 up.  */
#ifdef __ARMEB__
      "sub	r0, r0, r3, lsr #24\n\t"
#else
      "and	r3, r3, #255\n\t"
#ifdef __thumb2__
      /* No RSB instruction in Thumb2 */
      "lsr	r0, r0, #24\n\t"
      "sub	r0, r0, r3\n\t"
#else
      "rsb	r0, r3, r0, lsr #24\n\t"
#endif
#endif
#ifndef __thumb2__
      "ldr	r4, [sp], #4\n\t"
#endif
      "RETURN"
#elif (defined (__thumb__) && !defined (__thumb2__))
  "1:\n\t"
      "ldrb	r2, [r0]\n\t"
      "ldrb	r3, [r1]\n\t"
      "add	r0, r0, #1\n\t"
      "add	r1, r1, #1\n\t"
      "cmp	r2, #0\n\t"
      "beq	2f\n\t"
      "cmp	r2, r3\n\t"
      "beq	1b\n\t"
  "2:\n\t"
      "sub	r0, r2, r3\n\t"
      "bx	lr"
#else
 "3:\n\t"
      "ldrb	r2, [r0], #1\n\t"
      "ldrb	r3, [r1], #1\n\t"
      "cmp	r2, #1\n\t"
      "it	cs\n\t"
      "cmpcs	r2, r3\n\t"
      "beq	3b\n\t"
      "sub	r0, r2, r3\n\t"
      "RETURN"
#endif
      );
}

#if !(defined(__OPTIMIZE_SIZE__) || defined (PREFER_SIZE_OVER_SPEED) || \
      (defined (__thumb__) && !defined (__thumb2__)))
static int __attribute__((naked, used)) 
strcmp_unaligned(const char* s1, const char* s2)
{
#if 0
  /* The assembly code below is based on the following alogrithm.  */
#ifdef __ARMEB__
#define RSHIFT <<
#define LSHIFT >>
#else
#define RSHIFT >>
#define LSHIFT <<
#endif

#define body(shift)							\
  mask = 0xffffffffU RSHIFT shift;					\
  w1 = *wp1++;								\
  w2 = *wp2++;								\
  do									\
    {									\
      t1 = w1 & mask;							\
      if (__builtin_expect(t1 != w2 RSHIFT shift, 0))			\
	{								\
	  w2 RSHIFT= shift;						\
	  break;							\
	}								\
      if (__builtin_expect(((w1 - b1) & ~w1) & (b1 << 7), 0))		\
	{								\
	  /* See comment in assembler below re syndrome on big-endian */\
	  if ((((w1 - b1) & ~w1) & (b1 << 7)) & mask)			\
	    w2 RSHIFT= shift;						\
	  else								\
	    {								\
	      w2 = *wp2;						\
	      t1 = w1 RSHIFT (32 - shift);				\
	      w2 = (w2 LSHIFT (32 - shift)) RSHIFT (32 - shift);	\
	    }								\
	  break;							\
	}								\
      w2 = *wp2++;							\
      t1 ^= w1;								\
      if (__builtin_expect(t1 != w2 LSHIFT (32 - shift), 0))		\
	{								\
	  t1 = w1 >> (32 - shift);					\
	  w2 = (w2 << (32 - shift)) RSHIFT (32 - shift);		\
	  break;							\
	}								\
      w1 = *wp1++;							\
    } while (1)

  const unsigned* wp1;
  const unsigned* wp2;
  unsigned w1, w2;
  unsigned mask;
  unsigned shift;
  unsigned b1 = 0x01010101;
  char c1, c2;
  unsigned t1;

  while (((unsigned) s1) & 3)
    {
      c1 = *s1++;
      c2 = *s2++;
      if (c1 == 0 || c1 != c2)
	return c1 - (int)c2;
    }
  wp1 = (unsigned*) (((unsigned)s1) & ~3);
  wp2 = (unsigned*) (((unsigned)s2) & ~3);
  t1 = ((unsigned) s2) & 3;
  if (t1 == 1)
    {
      body(8);
    }
  else if (t1 == 2)
    {
      body(16);
    }
  else
    {
      body (24);
    }
  
  do
    {
#ifdef __ARMEB__
      c1 = (char) t1 >> 24;
      c2 = (char) w2 >> 24;
#else
      c1 = (char) t1;
      c2 = (char) w2;
#endif
      t1 RSHIFT= 8;
      w2 RSHIFT= 8;
    } while (c1 != 0 && c1 == c2);
  return c1 - c2;
#endif

  asm("wp1 .req r0\n\t"
      "wp2 .req r1\n\t"
      "b1  .req r2\n\t"
      "w1  .req r4\n\t"
      "w2  .req r5\n\t"
      "t1  .req ip\n\t"
      "@ r3 is scratch\n"

      /* First of all, compare bytes until wp1(sp1) is word-aligned. */
 "1:\n\t"
      "tst	wp1, #3\n\t"
      "beq	2f\n\t"
      "ldrb	r2, [wp1], #1\n\t"
      "ldrb	r3, [wp2], #1\n\t"
      "cmp	r2, #1\n\t"
      "it	cs\n\t"
      "cmpcs	r2, r3\n\t"
      "beq	1b\n\t"
      "sub	r0, r2, r3\n\t"
      "RETURN\n"

 "2:\n\t"
      "str	r5, [sp, #-4]!\n\t"
      "str	r4, [sp, #-4]!\n\t"
      //      "stmfd	sp!, {r4, r5}\n\t"
      "mov	b1, #1\n\t"
      "orr	b1, b1, b1, lsl #8\n\t"
      "orr	b1, b1, b1, lsl #16\n\t"

      "and	t1, wp2, #3\n\t"
      "bic	wp2, wp2, #3\n\t"
      "ldr	w1, [wp1], #4\n\t"
      "ldr	w2, [wp2], #4\n\t"
      "cmp	t1, #2\n\t"
      "beq	2f\n\t"
      "bhi	3f\n"

      /* Critical inner Loop: Block with 3 bytes initial overlap */
      ".p2align	2\n"
 "1:\n\t"
      "bic	t1, w1, #"MSB"\n\t"
      "cmp	t1, w2, "SHFT2LSB" #8\n\t"
      "sub	r3, w1, b1\n\t"
      "bic	r3, r3, w1\n\t"
      "bne	4f\n\t"
      "ands	r3, r3, b1, lsl #7\n\t"
      "it	eq\n\t"
      "ldreq	w2, [wp2], #4\n\t"
      "bne	5f\n\t"
      "eor	t1, t1, w1\n\t"
      "cmp	t1, w2, "SHFT2MSB" #24\n\t"
      "bne	6f\n\t"
      "ldr	w1, [wp1], #4\n\t"
      "b	1b\n"
 "4:\n\t"
      SHFT2LSB"	w2, w2, #8\n\t"
      "b	8f\n"

 "5:\n\t"
#ifdef __ARMEB__
      /* The syndrome value may contain false ones if the string ends
	 with the bytes 0x01 0x00 */
      "tst	w1, #0xff000000\n\t"
      "itt	ne\n\t"
      "tstne	w1, #0x00ff0000\n\t"
      "tstne	w1, #0x0000ff00\n\t"
      "beq	7f\n\t"
#else
      "bics	r3, r3, #0xff000000\n\t"
      "bne	7f\n\t"
#endif
      "ldrb	w2, [wp2]\n\t"
      SHFT2LSB"	t1, w1, #24\n\t"
#ifdef __ARMEB__
      "lsl	w2, w2, #24\n\t"
#endif
      "b	8f\n"

 "6:\n\t"
      SHFT2LSB"	t1, w1, #24\n\t"
      "and	w2, w2, #"LSB"\n\t"
      "b	8f\n"

      /* Critical inner Loop: Block with 2 bytes initial overlap */
      ".p2align	2\n"
 "2:\n\t"
      SHFT2MSB"	t1, w1, #16\n\t"
      "sub	r3, w1, b1\n\t"
      SHFT2LSB"	t1, t1, #16\n\t"
      "bic	r3, r3, w1\n\t"
      "cmp	t1, w2, "SHFT2LSB" #16\n\t"
      "bne	4f\n\t"
      "ands	r3, r3, b1, lsl #7\n\t"
      "it	eq\n\t"
      "ldreq	w2, [wp2], #4\n\t"
      "bne	5f\n\t"
      "eor	t1, t1, w1\n\t"
      "cmp	t1, w2, "SHFT2MSB" #16\n\t"
      "bne	6f\n\t"
      "ldr	w1, [wp1], #4\n\t"
      "b	2b\n"

 "5:\n\t"
#ifdef __ARMEB__
      /* The syndrome value may contain false ones if the string ends
	 with the bytes 0x01 0x00 */
      "tst	w1, #0xff000000\n\t"
      "it	ne\n\t"
      "tstne	w1, #0x00ff0000\n\t"
      "beq	7f\n\t"
#else
      "lsls	r3, r3, #16\n\t"
      "bne	7f\n\t"
#endif
      "ldrh	w2, [wp2]\n\t"
      SHFT2LSB"	t1, w1, #16\n\t"
#ifdef __ARMEB__
      "lsl	w2, w2, #16\n\t"
#endif
      "b	8f\n"

 "6:\n\t"
      SHFT2MSB"	w2, w2, #16\n\t"
      SHFT2LSB"	t1, w1, #16\n\t"
 "4:\n\t"
      SHFT2LSB"	w2, w2, #16\n\t"
      "b	8f\n\t"

      /* Critical inner Loop: Block with 1 byte initial overlap */
      ".p2align	2\n"
 "3:\n\t"
      "and	t1, w1, #"LSB"\n\t"
      "cmp	t1, w2, "SHFT2LSB" #24\n\t"
      "sub	r3, w1, b1\n\t"
      "bic	r3, r3, w1\n\t"
      "bne	4f\n\t"
      "ands	r3, r3, b1, lsl #7\n\t"
      "it	eq\n\t"
      "ldreq	w2, [wp2], #4\n\t"
      "bne	5f\n\t"
      "eor	t1, t1, w1\n\t"
      "cmp	t1, w2, "SHFT2MSB" #8\n\t"
      "bne	6f\n\t"
      "ldr	w1, [wp1], #4\n\t"
      "b	3b\n"
 "4:\n\t"
      SHFT2LSB"	w2, w2, #24\n\t"
      "b	8f\n"
 "5:\n\t"
      /* The syndrome value may contain false ones if the string ends
	 with the bytes 0x01 0x00 */
      "tst	w1, #"LSB"\n\t"
      "beq	7f\n\t"
      "ldr	w2, [wp2], #4\n"
 "6:\n\t"
      SHFT2LSB"	t1, w1, #8\n\t"
      "bic	w2, w2, #"MSB"\n\t"
      "b	8f\n"
 "7:\n\t"
      "mov	r0, #0\n\t"
      //      "ldmfd	sp!, {r4, r5}\n\t"
      "ldr	r4, [sp], #4\n\t"
      "ldr	r5, [sp], #4\n\t"
      "RETURN\n"
 "8:\n\t"
      "and	r2, t1, #"LSB"\n\t"
      "and	r0, w2, #"LSB"\n\t"
      "cmp	r0, #1\n\t"
      "it	cs\n\t"
      "cmpcs	r0, r2\n\t"
      "itt	eq\n\t"
      SHFT2LSB"eq	t1, t1, #8\n\t"
      SHFT2LSB"eq	w2, w2, #8\n\t"
      "beq	8b\n\t"
      "sub	r0, r2, r0\n\t"
      //      "ldmfd	sp!, {r4, r5}\n\t"
      "ldr	r4, [sp], #4\n\t"
      "ldr	r5, [sp], #4\n\t"
      "RETURN");
}

#endif
