/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Adapted for use as log2 by Ulrich Drepper <drepper@cygnus.com>.
 * Public domain.
 *
 * Changed to use fyl2xp1 for values near 1, <drepper@cygnus.com>.
 *
 * Removed header file dependency for use in libmingwex.a by
 *   Danny Smith <dannysmith@users.sourceforge.net>
 */

	.file	"log2.S"
	.text
	.align 4
one:	.double 1.0
	/* It is not important that this constant is precise.  It is only
	   a value which is known to be on the safe side for using the
	   fyl2xp1 instruction.  */
limit:	.double 0.29

	.text
	.align 4
.globl _log2
	.def	_log2;	.scl	2;	.type	32;	.endef
_log2:
	fldl	one
	fldl	4(%esp)		/* x : 1 */
	fxam
	fnstsw
	fld	%st		/* x : x : 1 */
	sahf
	jc	3f		/* in case x is NaN or ±Inf */
4:	fsub	%st(2), %st	/* x-1 : x : 1 */
	fld	%st		/* x-1 : x-1 : x : 1 */
	fabs			/* |x-1| : x-1 : x : 1 */
	fcompl	limit		/* x-1 : x : 1 */
	fnstsw			/* x-1 : x : 1 */
	andb	$0x45, %ah
	jz	2f
	fstp	%st(1)		/* x-1 : 1 */
	fyl2xp1			/* log(x) */
	ret

2:	fstp	%st(0)		/* x : 1 */
	fyl2x			/* log(x) */
	ret

3:	jp	4b		/* in case x is ±Inf */
	fstp	%st(1)
	fstp	%st(1)
	ret
