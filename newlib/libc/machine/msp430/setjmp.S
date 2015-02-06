/* Copyright (c) 2013  Red Hat, Inc. All rights reserved.

   This copyrighted material is made available to anyone wishing to use,
   modify, copy, or redistribute it subject to the terms and conditions
   of the BSD License.   This program is distributed in the hope that
   it will be useful, but WITHOUT ANY WARRANTY expressed or implied,
   including the implied warranties of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  A copy of this license is available at
   http://www.opensource.org/licenses. Any Red Hat trademarks that are
   incorporated in the source code or documentation are not subject to
   the BSD License and may only be used or replicated with the express
   permission of Red Hat, Inc.
*/

# setjmp/longjmp for msp430.  The jmpbuf looks like this:
#
# Register	Jmpbuf offset
#               small   large	
# r0 (pc)	0x00     0x00
# r1 (sp)	0x02	 0x04	
# r4		0x04	 0x08
# r5		0x06	 0x0c
# r6		0x08	 0x10
# r7		0x0a	 0x14
# r8		0x0c	 0x18
# r9		0x0e	 0x1c
# r10		0x10	 0x20

	.text
	.global	setjmp
setjmp:
	; Upon entry r12 points to the jump buffer.
	; Returns 0 to caller.
	
#if   defined __MSP430X_LARGE__
	mova   @r1, r13
	mova    r13, 0(r12)
	mova	r1,  4(r12)
	mova	r4,  8(r12)
	mova	r5,  12(r12)
	mova	r6,  16(r12)
	mova	r7,  20(r12)
	mova	r8,  24(r12)
	mova	r9,  28(r12)
	mova	r10, 32(r12)
	clr   	r12
	reta
#else
	;; Get the return address off the stack
	mov.w  @r1,  r13
	mov.w   r13, 0(r12)
	mov.w	r1,  2(r12)
	mov.w	r4,  4(r12)
	mov.w	r5,  6(r12)
	mov.w	r6,  8(r12)
	mov.w	r7,  10(r12)
	mov.w	r8,  12(r12)
	mov.w	r9,  14(r12)
	mov.w	r10, 16(r12)
	clr   	r12
	ret
#endif	
	.size setjmp , . - setjmp

	
	.global	longjmp
longjmp:
	; Upon entry r12 points to the jump buffer and
        ; r13 contains the value to be returned by setjmp.

#if   defined __MSP430X_LARGE__
	mova	@r12+, r14
	mova	@r12+, r1
	mova	@r12+, r4
	mova	@r12+, r5
	mova	@r12+, r6
	mova	@r12+, r7
	mova	@r12+, r8
	mova	@r12+, r9
	mova	@r12+, r10
#else
	mov.w	@r12+, r14
	mov.w	@r12+, r1
	mov.w	@r12+, r4
	mov.w	@r12+, r5
	mov.w	@r12+, r6
	mov.w	@r12+, r7
	mov.w	@r12+, r8
	mov.w	@r12+, r9
	mov.w	@r12+, r10
#endif
	; If caller attempts to return 0, return 1 instead.
	cmp.w   #0, r13
	jne	.Lnot_zero
	mov.w	#1, r13
.Lnot_zero:
	mov.w	r13, r12

#if   defined __MSP430X_LARGE__
	adda     #4, r1
	mova	r14, r0
#else
	add.w    #2, r1
	mov.w	r14, r0
#endif
	.size longjmp , . - longjmp

