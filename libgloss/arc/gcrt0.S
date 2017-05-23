/*
   Copyright (c) 2016, Synopsys, Inc. All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   1) Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

   2) Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

   3) Neither the name of the Synopsys, Inc., nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
*/
#define PROFILE_SUPPORT 1

#include "crt0.S"

	.global __mcount
	.type	__mcount, @function
	.align 4
__mcount:
	;; When a function is compiled for profiling, gcc will insert
	;; a call to __mcount without checking for resources.  Hence,
	;; we need save all of the argument registers and temp registers,
	;; extract the address of <func_name>, and call the _mcount_internal
	;; to do the real work.  Finally, restore all the argument registers
	;; before returning.
	push_s	blink
	push_s	r0
	push_s	r1
	push_s	r2
	push_s	r3
#ifndef __ARC_RF16__
	push	r4
	push	r5
	push	r6
	push	r7
	push	r8
	push	r9
#endif
	push	r10
	push	r11
	push	r12
	mov	r0,blink
	jl	@_mcount_internal
	pop	r12
	pop	r11
	pop	r10
#ifndef __ARC_RF16__
	pop	r9
	pop	r8
	pop	r7
	pop	r6
	pop	r5
	pop	r4
#endif
	pop_s	r3
	pop_s	r2
	pop_s	r1
	pop_s	r0
	pop_s	blink
	j_s	[blink]
	.size __mcount, . - __mcount
