/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 * Adapted for float by Ulrich Drepper <drepper@cygnus.com>.
 *
 * Changed to use fyl2xp1 for values near 1, <drepper@cygnus.com>.
 */

	.file	"logf.S"
	.text
	.align 4
one:	.double 1.0
	/* It is not important that this constant is precise.  It is only
	   a value which is known to be on the safe side for using the
	   fyl2xp1 instruction.  */
limit:	.double 0.29

	.text
	.align 4
.globl _logf
	.def	_logf;	.scl	2;	.type	32;	.endef
_logf:
	fldln2			/* log(2) */
	flds	4(%esp)		/* x : log(2) */
	fld	%st		/* x : x : log(2) */
	fsubl	one		/* x-1 : x : log(2) */
	fld	%st		/* x-1 : x-1 : x : log(2) */
	fabs			/* |x-1| : x-1 : x : log(2) */
	fcompl	limit		/* x-1 : x : log(2) */
	fnstsw			/* x-1 : x : log(2) */
	andb	$0x45, %ah
	jz	2f
	fstp	%st(1)		/* x-1 : log(2) */
	fyl2xp1			/* log(x) */
	ret

2:	fstp	%st(0)		/* x : log(2) */
	fyl2x			/* log(x) */
	ret
