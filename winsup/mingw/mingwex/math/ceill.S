/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 * Changes for long double by Ulrich Drepper <drepper@cygnus.com>
 */


	.file	"ceill.S"
	.text
	.align 4
.globl _ceill
	.def	_ceill;	.scl	2;	.type	32;	.endef
_ceill:
	fldt	4(%esp)
	subl	$8,%esp

	fstcw	4(%esp)			/* store fpu control word */

	/* We use here %edx although only the low 1 bits are defined.
	   But none of the operations should care and they are faster
	   than the 16 bit operations.  */
	movl	$0x0800,%edx		/* round towards +oo */
	orl	4(%esp),%edx
	andl	$0xfbff,%edx
	movl	%edx,(%esp)
	fldcw	(%esp)			/* load modified control word */

	frndint				/* round */

	fldcw	4(%esp)			/* restore original control word */

	addl	$8,%esp
	ret
