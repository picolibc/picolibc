/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 * Adapted for float type by Ulrich Drepper <drepper@cygnus.com>.
 *
 * Changed to use fyl2xp1 for values near 1, <drepper@cygnus.com>.
 */

	.file	"log10f.S"
	.text
	.align 4
one:	.double 1.0
	/* It is not important that this constant is precise.  It is only
	   a value which is known to be on the safe side for using the
	   fyl2xp1 instruction.  */
limit:	.double 0.29

	.text
	.align 4
.globl _log10f
	.def	_log10f;	.scl	2;	.type	32;	.endef
_log10f:
	fldlg2			/* log10(2) */
	flds	4(%esp)		/* x : log10(2) */
	fxam
	fnstsw
	fld	%st		/* x : x : log10(2) */
	sahf
	jc	3f		/* in case x is NaN or ±Inf */
4:	fsubl	one		/* x-1 : x : log10(2) */
	fld	%st		/* x-1 : x-1 : x : log10(2) */
	fabs			/* |x-1| : x-1 : x : log10(2) */
	fcompl	limit		/* x-1 : x : log10(2) */
	fnstsw			/* x-1 : x : log10(2) */
	andb	$0x45, %ah
	jz	2f
	fstp	%st(1)		/* x-1 : log10(2) */
	fyl2xp1			/* log10(x) */
	ret

2:	fstp	%st(0)		/* x : log10(2) */
	fyl2x			/* log10(x) */
	ret

3:	jp	4b		/* in case x is ±Inf */
	fstp	%st(1)
	fstp	%st(1)
	ret
