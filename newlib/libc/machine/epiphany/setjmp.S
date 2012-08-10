/* setjmp and longjmp

   Copyright (c) 2011, Adapteva, Inc.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Adapteva nor the names of its contributors may be
      used to endorse or promote products derived from this software without
      specific prior written permission.

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
   POSSIBILITY OF SUCH DAMAGE.                                               */

	.file "setjmp.S"
        .section .text
        .align 4
	.global  _setjmp
        .type _setjmp, %function
_setjmp:
	strd lr,[r0]
	strd r4,[r0,1]
	strd r6,[r0,2]
	strd r8,[r0,3]
	strd r10,[r0,4]
	strd r32,[r0,5]
	strd r34,[r0,6]
	strd r36,[r0,7]
	strd r38,[r0,8]
	str sp,[r0,18]
	mov r0,#0
	rts
	.size _setjmp, .-_setjmp

	.global _longjmp
_longjmp:
	ldrd lr,[r0] ; return address / r15
	ldrd r4,[r0,1]
	ldrd r6,[r0,2]
	ldrd r8,[r0,3]
	ldrd r10,[r0,4]
	ldrd r32,[r0,5]
	ldrd r34,[r0,6]
	ldrd r36,[r0,7]
	ldrd r38,[r0,8]
	ldr sp,[r0,18]
	sub r1,r1,0
	mov r0,#1
	movne r0,r1
	jr lr
        .size   _longjmp, .-_longjmp
