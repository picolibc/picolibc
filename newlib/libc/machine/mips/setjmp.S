/* This is a simple version of setjmp and longjmp.

   This version does NOT save the floating point register, which is
   wrong, but I don't know how to cleanly handle machines without a
   floating point coprocessor.

   Ian Lance Taylor, Cygnus Support, 13 May 1993.  */

#ifdef __mips16
/* This file contains 32 bit assembly code.  */
	.set nomips16
#endif

/* int setjmp (jmp_buf);  */
	.globl	setjmp
	.ent	setjmp
setjmp:
	.frame	$sp,0,$31

	sw	$16,0($4)	/* $s0 */
	sw	$17,4($4)	/* $s1 */
	sw	$18,8($4)	/* $s2 */
	sw	$19,12($4)	/* $s3 */
	sw	$20,16($4)	/* $s4 */
	sw	$21,20($4)	/* $s5 */
	sw	$22,24($4)	/* $s6 */
	sw	$23,28($4)	/* $s7 */
	sw	$30,32($4)	/* $s8 */

	sw	$sp,36($4)
	sw	$31,40($4)

	move	$2,$0

	j	$31

	.end	setjmp

/* volatile void longjmp (jmp_buf, int);  */
	.globl	longjmp
	.ent	longjmp
longjmp:
	.frame	$sp,0,$31

	lw	$16,0($4)	/* $s0 */
	lw	$17,4($4)	/* $s1 */
	lw	$18,8($4)	/* $s2 */
	lw	$19,12($4)	/* $s3 */
	lw	$20,16($4)	/* $s4 */
	lw	$21,20($4)	/* $s5 */
	lw	$22,24($4)	/* $s6 */
	lw	$23,28($4)	/* $s7 */
	lw	$30,32($4)	/* $s8 */

	lw	$sp,36($4)
	lw	$31,40($4)

	bne	$5,$0,1f
	li	$5,1
1:
	move	$2,$5

	j	$31

	.end longjmp
