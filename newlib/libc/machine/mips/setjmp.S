/* This is a simple version of setjmp and longjmp for MIPS 32 and 64.

   The MIPS 32 version does NOT save the floating point register, which is
   wrong, but I don't know how to cleanly handle machines without a
   floating point coprocessor.

   Ian Lance Taylor, Cygnus Support, 13 May 1993.

   The MIPS 64 version saves registers fp20 to fp31.  23 registers
   are saved in all.  */

#ifndef __mips64
#ifdef __mips16
/* This file contains 32 bit assembly code.  */
	.set nomips16
#endif
#endif

/* int setjmp (jmp_buf);  */
	.globl	setjmp
	.ent	setjmp
setjmp:
	.frame	$sp,0,$31

#ifdef __mips64

	sd      $16, 000($4)    /* s0 */
	sd      $17, 010($4)    /* s1 */
	sd      $18, 020($4)    /* s2 */
	sd      $19, 030($4)    /* s3 */
	sd      $20, 040($4)    /* s4 */
	sd      $21, 050($4)    /* s5 */
	sd      $22, 060($4)    /* s6 */
	sd      $23, 070($4)    /* s7 */

	sdc1    $f20, 0100($4)
	sdc1    $f21, 0110($4)
	sdc1    $f22, 0120($4)
	sdc1    $f23, 0130($4)
	sdc1    $f24, 0140($4)
	sdc1    $f25, 0150($4)
	sdc1    $f26, 0160($4)
	sdc1    $f27, 0170($4)
	sdc1    $f28, 0200($4)
	sdc1    $f29, 0210($4)
	sdc1    $f30, 0220($4)
	sdc1    $f31, 0230($4)

	sd      $29, 0240($4)   /* sp */
	sd      $30, 0250($4)   /* fp */
	sd      $31, 0260($4)   /* ra */

#else /* not __mips64 */

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

#endif /* not __mips64 */

	move	$2,$0

	j	$31

	.end	setjmp

/* volatile void longjmp (jmp_buf, int);  */
	.globl	longjmp
	.ent	longjmp
longjmp:
	.frame	$sp,0,$31

#ifdef __mips64

	ld      $16, 000($4)    /* s0 */
	ld      $17, 010($4)    /* s1 */
	ld      $18, 020($4)    /* s2 */
	ld      $19, 030($4)    /* s3 */
	ld      $20, 040($4)    /* s4 */
	ld      $21, 050($4)    /* s5 */
	ld      $22, 060($4)    /* s6 */
	ld      $23, 070($4)    /* s7 */

	ldc1    $f20, 0100($4)
	ldc1    $f21, 0110($4)
	ldc1    $f22, 0120($4)
	ldc1    $f23, 0130($4)
	ldc1    $f24, 0140($4)
	ldc1    $f25, 0150($4)
	ldc1    $f26, 0160($4)
	ldc1    $f27, 0170($4)
	ldc1    $f28, 0200($4)
	ldc1    $f29, 0210($4)
	ldc1    $f30, 0220($4)
	ldc1    $f31, 0230($4)

	ld      $29, 0240($4)   /* sp */
	ld      $30, 0250($4)   /* fp */
	ld      $31, 0260($4)   /* ra */

#else /* not __mips64 */

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

#endif /* not __mips64 */

	bne	$5,$0,1f
	li	$5,1
1:
	move	$2,$5

	j	$31

	.end longjmp
