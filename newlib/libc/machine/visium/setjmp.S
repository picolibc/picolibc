/* setjmp/longjmp for the Visium processor.

   Copyright (c) 2015 Rolls-Royce Controls and Data Services Limited.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

     * Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of Rolls-Royce Controls and Data Services Limited nor
       the names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
   THE POSSIBILITY OF SUCH DAMAGE.  */

	.text
	.globl setjmp
	.type setjmp, @function
setjmp:
	write.l	0(r1),r11
	write.l	1(r1),r12
	write.l	2(r1),r13
	write.l	3(r1),r14
	write.l	4(r1),r15
	write.l	5(r1),r16
	write.l	6(r1),r17
	write.l	7(r1),r18
	write.l	8(r1),r19
	write.l	9(r1),r21
	write.l	10(r1),r22
	write.l	11(r1),r23
	bra 	tr,r21,r0
	 moviq	r1,0
	.size setjmp, .-setjmp

	.globl longjmp
	.type longjmp, @function
longjmp:
	read.l	r11,0(r1)
	read.l	r12,1(r1)
	read.l	r13,2(r1)
	read.l	r14,3(r1)
	read.l	r15,4(r1)
	read.l	r16,5(r1)
	read.l	r17,6(r1)
	read.l	r18,7(r1)
	read.l	r19,8(r1)
	read.l	r21,9(r1)
	read.l	r22,10(r1)
	read.l	r23,11(r1)
	bra	tr,r21,r0
	 move.l	r1,r2
	.size longjmp, .-longjmp
