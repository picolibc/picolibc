/* Copyright (c) 2015-2016, ARM Limited
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
       * Redistributions of source code must retain the above copyright
         notice, this list of conditions and the following disclaimer.
       * Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.
       * Neither the name of the company nor the names of its contributors
         may be used to endorse or promote products derived from this
         software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  */

/* Assumptions:
 *
 * ARMv8-a, AArch64, unaligned accesses
 *
 */

#if (defined (__OPTIMIZE_SIZE__) || defined (PREFER_SIZE_OVER_SPEED))
/* See rawmemchr-stub.c.  */
#else

#define L(l) .L ## l

	.macro def_fn f p2align=0
	.text
	.p2align \p2align
	.global \f
	.type \f, %function
\f:
	.endm

/* Special case rawmemchr (s, 0) as strlen, otherwise tailcall memchr.
   Call strlen without setting up a full frame - it preserves x14/x15.
*/

def_fn rawmemchr p2align=5
	.cfi_startproc
	cbz	w1, L(do_strlen)
	mov	x2, -1
	b	memchr

L(do_strlen):
	mov	x15, x30
	.cfi_return_column x15
	mov	x14, x0
	bl	strlen
	add	x0, x14, x0
	ret	x15
	.cfi_endproc

	.size   rawmemchr, . - rawmemchr
#endif

