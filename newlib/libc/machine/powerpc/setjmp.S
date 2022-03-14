/*
Copyright (c) 1990 The Regents of the University of California.
All rights reserved.

Redistribution and use in source and binary forms are permitted
provided that the above copyright notice and this paragraph are
duplicated in all such forms and that any documentation,
and/or other materials related to such
distribution and use acknowledge that the software was developed
by the University of California, Berkeley.  The name of the
University may not be used to endorse or promote products derived
from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/* This is a simple version of setjmp and longjmp for the PowerPC.
   Ian Lance Taylor, Cygnus Support, 9 Feb 1994.  
   Modified by Jeff Johnston, Red Hat Inc. 2 Oct 2001. */

#include "ppc-asm.h"

#ifdef _LP64
#define STW	std
#define STWU	stdu
#define LWZ	ld
#define LWZU	ldu
#define OFFSET	8
#else
#define STW	stw
#define STWU	stwu
#define LWZ	lwz
#define LWZU	lwzu
#define OFFSET	4
#endif

FUNC_START(setjmp)
#ifdef __ALTIVEC__
	addi	3,3,15		# align Altivec to 16 byte boundary
#ifdef _LP64
	rldicr	3,3,0,59
#else
	rlwinm	3,3,0,0,27
#endif
#else
	addi	3,3,7		# align to 8 byte boundary
#ifdef _LP64
	rldicr	3,3,0,60
#else
	rlwinm	3,3,0,0,28
#endif	
#endif
#if __SPE__
	/* If we are E500, then save 64-bit registers.  */
	evstdd	1,0(3)		# offset 0
	evstdd	2,8(3)		# offset 8
	evstdd	13,16(3)	# offset 16
	evstdd	14,24(3)	# offset 24
	evstdd	15,32(3)	# offset 32
	evstdd	16,40(3)	# offset 40
	evstdd	17,48(3)	# offset 48
	evstdd	18,56(3)	# offset 56
	evstdd	19,64(3)	# offset 64
	evstdd	20,72(3)	# offset 72
	evstdd	21,80(3)	# offset 80
	evstdd	22,88(3)	# offset 88
	evstdd	23,96(3)	# offset 96
	evstdd	24,104(3)	# offset 104
	evstdd	25,112(3)	# offset 112
	evstdd	26,120(3)	# offset 120
	evstdd	27,128(3)	# offset 128
	evstdd	28,136(3)	# offset 136
	evstdd	29,144(3)	# offset 144
	evstdd	30,152(3)	# offset 152
	evstdd	31,160(3)	# offset 160

	/* Add 164 to r3 to account for the amount of data we just
	   stored.  Note that we are not adding 168 because the next
	   store instruction uses an offset of 4.  */
	addi	3,3,164
#else
	STW	1,0(3)		# offset 0
	STWU	2,OFFSET(3)	# offset 4
	STWU	13,OFFSET(3)	# offset 8
	STWU	14,OFFSET(3)	# offset 12
	STWU	15,OFFSET(3)	# offset 16
	STWU	16,OFFSET(3)	# offset 20
	STWU	17,OFFSET(3)	# offset 24
	STWU	18,OFFSET(3)	# offset 28
	STWU	19,OFFSET(3)	# offset 32
	STWU	20,OFFSET(3)	# offset 36
	STWU	21,OFFSET(3)	# offset 40
	STWU	22,OFFSET(3)	# offset 44
	STWU	23,OFFSET(3)	# offset 48
	STWU	24,OFFSET(3)	# offset 52
	STWU	25,OFFSET(3)	# offset 56
	STWU	26,OFFSET(3)	# offset 60
	STWU	27,OFFSET(3)	# offset 64
	STWU	28,OFFSET(3)	# offset 68
	STWU	29,OFFSET(3)	# offset 72
	STWU	30,OFFSET(3)	# offset 76
	STWU	31,OFFSET(3)	# offset 80
#endif

	/* From this point on until the end of this function, add 84
	   to the offset shown if __SPE__.  This difference comes from
	   the fact that we save 21 64-bit registers instead of 21
	   32-bit registers above.  */
	mflr	4
	STWU	4,OFFSET(3)	# offset 84
	mfcr	4
	STWU	4,OFFSET(3)	# offset 88
				# one word pad to get floating point aligned on 8 byte boundary

	/* Check whether we need to save FPRs.  Checking __NO_FPRS__
	   on its own would be enough for GCC 4.1 and above, but older
	   compilers only define _SOFT_FLOAT, so check both.  */
#if !defined (__NO_FPRS__) && !defined (_SOFT_FLOAT)
#if defined (__rtems__) && !defined (__PPC_CPU_E6500__)
	/* For some RTEMS multilibs, the FPU and Altivec units are disabled
	   during interrupt handling.  Do not save and restore the
	   corresponding registers in this case.  */
	mfmsr	5
	andi.	5,5,0x2000
	beq	1f
#endif
	stfdu	14,8(3)		# offset 96
	stfdu	15,8(3)		# offset 104
	stfdu	16,8(3)		# offset 112
	stfdu	17,8(3)		# offset 120
	stfdu	18,8(3)		# offset 128
	stfdu	19,8(3)		# offset 136
	stfdu	20,8(3)		# offset 144
	stfdu	21,8(3)		# offset 152
	stfdu	22,8(3)		# offset 160
	stfdu	23,8(3)		# offset 168
	stfdu	24,8(3)		# offset 176
	stfdu	25,8(3)		# offset 184
	stfdu	26,8(3)		# offset 192
	stfdu	27,8(3)		# offset 200
	stfdu	28,8(3)		# offset 208
	stfdu	29,8(3)		# offset 216
	stfdu	30,8(3)		# offset 224
	stfdu	31,8(3)		# offset 232
1:
#endif

	/* This requires a total of 21 * 4 + 18 * 8 + 4 + 4 + 4
	   bytes == 60 * 4 bytes == 240 bytes.  */

#ifdef __ALTIVEC__
#if defined (__rtems__) && !defined (__PPC_CPU_E6500__)
	mfmsr	5
	andis.	5,5,0x200
	beq	1f
#endif
	/* save Altivec vrsave and vr20-vr31 registers */
	mfspr	4,256		# vrsave register
	STWU	4,16(3)		# offset 248
	addi	3,3,8
	stvx	20,0,3		# offset 256
	addi	3,3,16
	stvx	21,0,3		# offset 272
	addi	3,3,16
	stvx	22,0,3		# offset 288
	addi	3,3,16
	stvx	23,0,3		# offset 304
	addi	3,3,16
	stvx	24,0,3		# offset 320
	addi	3,3,16
	stvx	25,0,3		# offset 336
	addi	3,3,16
	stvx	26,0,3		# offset 352
	addi	3,3,16
	stvx	27,0,3		# offset 368
	addi	3,3,16
	stvx	28,0,3		# offset 384
	addi	3,3,16
	stvx	29,0,3		# offset 400
	addi	3,3,16
	stvx	30,0,3		# offset 416
	addi	3,3,16
	stvx	31,0,3		# offset 432
1:

	/* This requires a total of 240 + 8 + 8 + 12 * 16 == 448 bytes. */
#endif
	li	3,0
	blr
FUNC_END(setjmp)


FUNC_START(longjmp)
#ifdef __ALTIVEC__
	addi	3,3,15		# align Altivec to 16 byte boundary
#ifdef _LP64
	rldicr	3,3,0,59
#else
	rlwinm	3,3,0,0,27
#endif
#else
	addi	3,3,7		# align to 8 byte boundary
#ifdef _LP64
	rldicr	3,3,0,60
#else
	rlwinm	3,3,0,0,28
#endif	
#endif
#if __SPE__
	/* If we are E500, then restore 64-bit registers.  */
	evldd	1,0(3)		# offset 0
	evldd	2,8(3)		# offset 8
	evldd	13,16(3)	# offset 16
	evldd	14,24(3)	# offset 24
	evldd	15,32(3)	# offset 32
	evldd	16,40(3)	# offset 40
	evldd	17,48(3)	# offset 48
	evldd	18,56(3)	# offset 56
	evldd	19,64(3)	# offset 64
	evldd	20,72(3)	# offset 72
	evldd	21,80(3)	# offset 80
	evldd	22,88(3)	# offset 88
	evldd	23,96(3)	# offset 96
	evldd	24,104(3)	# offset 104
	evldd	25,112(3)	# offset 112
	evldd	26,120(3)	# offset 120
	evldd	27,128(3)	# offset 128
	evldd	28,136(3)	# offset 136
	evldd	29,144(3)	# offset 144
	evldd	30,152(3)	# offset 152
	evldd	31,160(3)	# offset 160

	/* Add 164 to r3 to account for the amount of data we just
	   loaded.  Note that we are not adding 168 because the next
	   load instruction uses an offset of 4.  */
	addi	3,3,164
#else
	LWZ	1,0(3)		# offset 0 
	LWZU	2,OFFSET(3)	# offset 4 
	LWZU	13,OFFSET(3)	# offset 8 
	LWZU	14,OFFSET(3)	# offset 12
	LWZU	15,OFFSET(3)	# offset 16
	LWZU	16,OFFSET(3)	# offset 20
	LWZU	17,OFFSET(3)	# offset 24
	LWZU	18,OFFSET(3)	# offset 28
	LWZU	19,OFFSET(3)	# offset 32
	LWZU	20,OFFSET(3)	# offset 36
	LWZU	21,OFFSET(3)	# offset 40
	LWZU	22,OFFSET(3)	# offset 44
	LWZU	23,OFFSET(3)	# offset 48
	LWZU	24,OFFSET(3)	# offset 52
	LWZU	25,OFFSET(3)	# offset 56
	LWZU	26,OFFSET(3)	# offset 60
	LWZU	27,OFFSET(3)	# offset 64
	LWZU	28,OFFSET(3)	# offset 68
	LWZU	29,OFFSET(3)	# offset 72
	LWZU	30,OFFSET(3)	# offset 76
	LWZU	31,OFFSET(3)	# offset 80
#endif
	/* From this point on until the end of this function, add 84
	   to the offset shown if __SPE__.  This difference comes from
	   the fact that we restore 21 64-bit registers instead of 21
	   32-bit registers above.  */
	LWZU	5,OFFSET(3)	# offset 84
	mtlr	5
	LWZU	5,OFFSET(3)	# offset 88
	mtcrf	255,5
				# one word pad to get floating point aligned on 8 byte boundary

	/* Check whether we need to restore FPRs.  Checking
	   __NO_FPRS__ on its own would be enough for GCC 4.1 and
	   above, but older compilers only define _SOFT_FLOAT, so
	   check both.  */
#if !defined (__NO_FPRS__) && !defined (_SOFT_FLOAT)
#if defined (__rtems__) && !defined (__PPC_CPU_E6500__)
	mfmsr	5
	andi.	5,5,0x2000
	beq	1f
#endif
	lfdu	14,8(3)         # offset 96 
	lfdu	15,8(3)         # offset 104
	lfdu	16,8(3)         # offset 112
	lfdu	17,8(3)         # offset 120
	lfdu	18,8(3)         # offset 128
	lfdu	19,8(3)         # offset 136
	lfdu	20,8(3)         # offset 144
	lfdu	21,8(3)         # offset 152
	lfdu	22,8(3)         # offset 160
	lfdu	23,8(3)         # offset 168
	lfdu	24,8(3)         # offset 176
	lfdu	25,8(3)         # offset 184
	lfdu	26,8(3)         # offset 192
	lfdu	27,8(3)         # offset 200
	lfdu	28,8(3)         # offset 208
	lfdu	29,8(3)         # offset 216
	lfdu	30,8(3)         # offset 224
	lfdu	31,8(3)         # offset 232
1:
#endif

#ifdef __ALTIVEC__
#if defined (__rtems__) && !defined (__PPC_CPU_E6500__)
	mfmsr	5
	andis.	5,5,0x200
	beq	1f
#endif
	/* restore Altivec vrsave and v20-v31 registers */
	LWZU	5,16(3)		# offset 248
	mtspr	256,5		# vrsave
	addi	3,3,8
	lvx	20,0,3		# offset 256
	addi	3,3,16
	lvx	21,0,3		# offset 272
	addi	3,3,16
	lvx	22,0,3		# offset 288
	addi	3,3,16
	lvx	23,0,3		# offset 304
	addi	3,3,16
	lvx	24,0,3		# offset 320
	addi	3,3,16
	lvx	25,0,3		# offset 336
	addi	3,3,16
	lvx	26,0,3		# offset 352
	addi	3,3,16
	lvx	27,0,3		# offset 368
	addi	3,3,16
	lvx	28,0,3		# offset 384
	addi	3,3,16
	lvx	29,0,3		# offset 400
	addi	3,3,16
	lvx	30,0,3		# offset 416
	addi	3,3,16
	lvx	31,0,3		# offset 432
1:
#endif

	mr.	3,4
	bclr+	4,2
	li	3,1
	blr
FUNC_END(longjmp)
