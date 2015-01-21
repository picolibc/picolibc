/* crt0.S for the Visium processor.

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

	.weak _get_cmdline
	.text
	.align  4
	.global __start
__start:
	/* Initialize the stack pointer.  */
	moviu   r1,%u _estack
	movil   r1,%l _estack
	move.l  r23,r1

	/* Terminate the frame chain.  */
	moviq   r22,0

	/* Zero the .bss segment.  */
	moviu   r1,%u __bss_start
	movil   r1,%l __bss_start
	moviu   r2,%u __bss_end
	movil   r2,%l __bss_end
.L0:
	cmp.l   r1,r1
	brr     eq,.L1
	 nop
	write.b (r1),r0
	brr     tr,.L0
	 addi   r1,1
.L1:
	/* Register __fini (destructors) with atexit.  */
	moviu   r1,%u __fini
	moviu   r2,%u atexit
	movil   r2,%l atexit
	bra     tr,r2,r21
	 movil  r1,%l __fini

	/* Call __init (constructors).  */
	moviu   r1,%u __init
	movil   r1,%l __init
	bra     tr,r1,r21
	 nop

	/* Set up argc and argv.  */
	moviu   r3,%u _get_cmdline
	movil   r3,%l _get_cmdline
	cmp.l   r3,r0
	moviq   r1,0
	bra     ne,r3,r21
	 moviq  r2,0

	/* Call the main program.  */
	moviu   r3,%u main
	movil   r3,%l main
	bra     tr,r3,r21
	 nop

	/* Call exit in case the main program didn't.  */
	moviu   r2,%u exit
	movil   r2,%l exit
	bra     tr,r2,r21
	 nop
	.end
