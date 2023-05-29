/*
 * Copyright (c) 2012-2014 ARM Ltd
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

/* Wrapper for the various implementations of strcmp.  */

#include "machine/acle-compat.h"

#ifdef __ARM_BIG_ENDIAN
#define S2LO lsl
#define S2LOEQ lsleq
#define S2HI lsr
#define MSB 0x000000ff
#define LSB 0xff000000
#define BYTE0_OFFSET 24
#define BYTE1_OFFSET 16
#define BYTE2_OFFSET 8
#define BYTE3_OFFSET 0
#else /* not  __ARM_BIG_ENDIAN */
#define S2LO lsr
#define S2LOEQ lsreq
#define S2HI lsl
#define BYTE0_OFFSET 0
#define BYTE1_OFFSET 8
#define BYTE2_OFFSET 16
#define BYTE3_OFFSET 24
#define MSB 0xff000000
#define LSB 0x000000ff
#endif /* not  __ARM_BIG_ENDIAN */

	.macro def_fn f p2align=0
	.text
	.p2align \p2align
	.global \f
	.type \f, %function
\f:
	.endm

#if defined (__OPTIMIZE_SIZE__) || defined (PREFER_SIZE_OVER_SPEED) \
	|| (__ARM_ARCH_ISA_THUMB == 1 && !__ARM_ARCH_ISA_ARM)

# if defined (__thumb__) && !defined (__thumb2__)
/* Thumb1 only variant.  If size is preferred, use strcmp-armv4t.S.
   If speed is preferred, the strcmp function in strcmp-armv6m.S
   will be used.  */

#  if defined (__OPTIMIZE_SIZE__) || defined (PREFER_SIZE_OVER_SPEED)
#   include "strcmp-armv4t.S"
#  else
#   include "strcmp-armv6m.S"
#  endif

# else
#  include "strcmp-arm-tiny.S"
# endif

#elif __ARM_ARCH_ISA_THUMB == 2

# ifdef __ARM_FEATURE_SIMD32
#  include "strcmp-armv7.S"
# else
#  include "strcmp-armv7m.S"
# endif

#elif __ARM_ARCH >= 6

# include "strcmp-armv6.S"

#else

# include "strcmp-armv4.S"

#endif
