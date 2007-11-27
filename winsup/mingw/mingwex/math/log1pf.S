/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 * Removed header file dependency for use in libmingwex.a by
 *   Danny Smith <dannysmith@users.sourceforge.net>
 */

	.file	"log1pf.S"
	.text
	.align 4
	/* The fyl2xp1 can only be used for values in
	   -1 + sqrt(2) / 2 <= x <= 1 - sqrt(2) / 2
	   0.29 is a safe value.
	 */
limit:	.float 0.29
one:	.float 1.0
/*
 * Use the fyl2xp1 function when the argument is in the range -0.29 to 0.29,
 * otherwise fyl2x with the needed extra computation.
 */
.globl _log1pf; 
	.def	_log1pf;	.scl	2;	.type	32;	.endef
_log1pf:
	fldln2
	flds	4(%esp)
	fxam
	fnstsw
	fld	%st
	sahf
	jc	3f		/* in case x is NaN or ±Inf */

4:	fabs
	fcomps	limit
	fnstsw
	sahf
	jc	2f
	fadds	one
	fyl2x
	ret

2:	fyl2xp1
	ret

3:	jp	4b		/* in case x is ±Inf */
	fstp	%st(1)
	fstp	%st(1)
	ret
