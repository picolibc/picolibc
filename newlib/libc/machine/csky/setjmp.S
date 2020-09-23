/* Copyright (c) 2020  C-SKY Microsystems All rights reserved.

   This copyrighted material is made available to anyone wishing to use,
   modify, copy, or redistribute it subject to the terms and conditions
   of the FreeBSD License.   This program is distributed in the hope that
   it will be useful, but WITHOUT ANY WARRANTY expressed or implied,
   including the implied warranties of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  A copy of this license is available at
   http://www.opensource.org/licenses.
*/

	.section	.text
	.align	3
	.globl	setjmp
	.type	setjmp,@function
	.globl	longjmp
	.type	longjmp,@function

setjmp:

#if defined(__CK801__)
	stw	r4, (r0, 0)
	stw    	r5, (r0, 4)
	stw    	r6, (r0, 8)
	stw    	r7, (r0, 12)
	stw    	r8, (r0, 16)
	stw    	r15, (r0, 20)
	stw    	sp, (r0, 24)
#elif defined(__CK802__)
	stm	r4-r11, (r0)
	stw    	r15, (r0, 32)
	stw    	sp, (r0, 36)
#else
	stm    	r4-r11, (r0)
	stw    	r15, (r0, 32)
	stw    	r16, (r0, 36)
	stw    	r17, (r0, 40)
	stw    	r26, (r0, 44)
	stw    	r27, (r0, 48)
	stw    	r28, (r0, 52)
	stw    	r29, (r0, 56)
	stw    	r30, (r0, 60)
	stw    	r31, (r0, 64)
	stw    	sp, (r0, 68)
#endif
	movi   	r0, 0
	rts

longjmp:
#if defined(__CK801__)
	ldw	r4, (r0, 0)
	ldw    	r5, (r0, 4)
	ldw    	r6, (r0, 8)
	ldw    	r7, (r0, 12)
	ldw    	r8, (r0, 16)
	ldw    	r15, (r0, 20)
	ldw    	sp, (r0, 24)
#elif defined(__CK802__)
	ldm	r4-r11, (r0)
	ldw    	r15, (r0, 32)
	ldw    	sp, (r0, 36)
#else
	ldm   	r4-r11, (r0)
	ldw    	r15, (r0, 32)
	ldw    	r16, (r0, 36)
	ldw    	r17, (r0, 40)
	ldw    	r26, (r0, 44)
	ldw    	r27, (r0, 48)
	ldw    	r28, (r0, 52)
	ldw    	r29, (r0, 56)
	ldw    	r30, (r0, 60)
	ldw    	r31, (r0, 64)
	ldw    	sp, (r0, 68)
#endif
	mov    	r0, r1
	cmpnei 	r1, 0
	bt     	1f
	movi   	r0, 1
1:	rts
