/* This is a simple version of setjmp and longjmp for the PowerPC.
   Ian Lance Taylor, Cygnus Support, 9 Feb 1994.  
   Modified by Jeff Johnston, Red Hat Inc. 2 Oct 2001. */

#include "ppc-asm.h"

FUNC_START(setjmp)
#ifdef __ALTIVEC__
	addi	3,3,15		# align Altivec to 16 byte boundary
	rlwinm	3,3,0,0,27
#else
	addi	3,3,7		# align to 8 byte boundary
	rlwinm	3,3,0,0,28
#endif
	stw	1,0(3)		# offset 0
	stwu	2,4(3)		# offset 4
	stwu	13,4(3)		# offset 8
	stwu	14,4(3)		# offset 12
	stwu	15,4(3)		# offset 16
	stwu	16,4(3)		# offset 20
	stwu	17,4(3)		# offset 24
	stwu	18,4(3)		# offset 28
	stwu	19,4(3)		# offset 32
	stwu	20,4(3)		# offset 36
	stwu	21,4(3)		# offset 40
	stwu	22,4(3)		# offset 44
	stwu	23,4(3)		# offset 48
	stwu	24,4(3)		# offset 52
	stwu	25,4(3)		# offset 56
	stwu	26,4(3)		# offset 60
	stwu	27,4(3)		# offset 64
	stwu	28,4(3)		# offset 68
	stwu	29,4(3)		# offset 72
	stwu	30,4(3)		# offset 76
	stwu	31,4(3)		# offset 80
	mflr	4
	stwu	4,4(3)		# offset 84
	mfcr	4
	stwu	4,4(3)		# offset 88
				# one word pad to get floating point aligned on 8 byte boundary
#ifndef _SOFT_FLOAT
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
#endif

	/* This requires a total of 21 * 4 + 18 * 8 + 4 + 4 + 4
	   bytes == 60 * 4 bytes == 240 bytes.  */

#ifdef __ALTIVEC__
	/* save Altivec vrsave and vr20-vr31 registers */
	mfspr	4,256		# vrsave register
	stwu	4,16(3)		# offset 248
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

	/* This requires a total of 240 + 8 + 8 + 12 * 16 == 448 bytes. */
#endif
	li	3,0
	blr
FUNC_END(setjmp)


FUNC_START(longjmp)
#ifdef __ALTIVEC__
	addi	3,3,15		# align Altivec to 16 byte boundary
	rlwinm	3,3,0,0,27
#else
	addi	3,3,7		# align to 8 byte boundary
	rlwinm	3,3,0,0,28
#endif
	lwz	1,0(3)		# offset 0 
	lwzu	2,4(3)		# offset 4 
	lwzu	13,4(3)		# offset 8 
	lwzu	14,4(3)		# offset 12
	lwzu	15,4(3)		# offset 16
	lwzu	16,4(3)		# offset 20
	lwzu	17,4(3)		# offset 24
	lwzu	18,4(3)		# offset 28
	lwzu	19,4(3)		# offset 32
	lwzu	20,4(3)		# offset 36
	lwzu	21,4(3)		# offset 40
	lwzu	22,4(3)		# offset 44
	lwzu	23,4(3)		# offset 48
	lwzu	24,4(3)		# offset 52
	lwzu	25,4(3)		# offset 56
	lwzu	26,4(3)		# offset 60
	lwzu	27,4(3)		# offset 64
	lwzu	28,4(3)		# offset 68
	lwzu	29,4(3)		# offset 72
	lwzu	30,4(3)		# offset 76
	lwzu	31,4(3)		# offset 80
	lwzu	5,4(3)		# offset 84
	mtlr	5
	lwzu	5,4(3)		# offset 88
	mtcrf	255,5
				# one word pad to get floating point aligned on 8 byte boundary
#ifndef _SOFT_FLOAT
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
#endif

#ifdef __ALTIVEC__
	/* restore Altivec vrsave and v20-v31 registers */
	lwzu	5,16(3)		# offset 248
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
#endif

	mr.	3,4
	bclr+	4,2
	li	3,1
	blr
FUNC_END(longjmp)
