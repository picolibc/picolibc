/*
  Cephes Math Library Release 2.7:  May, 1998
  Copyright 1984, 1987, 1988, 1992, 1998 by Stephen L. Moshier

  Extracted from floorl.387 for use in libmingwex.a by
  Danny Smith <dannysmith@users.sourceforge.net>
  2002-06-20
*/

/*
 * frexpl(long double x, int* expnt) extracts the exponent from x.
 * It returns an integer power of two to expnt and the significand
 * between 0.5 and 1 to y.  Thus  x = y * 2**expn.
 */ 
	.align 2
.globl _frexpl
_frexpl:
	pushl %ebp
	movl %esp,%ebp
	subl $24,%esp
	pushl %esi
	pushl %ebx
	fldt 8(%ebp)
	movl 20(%ebp),%ebx
	fld %st(0)
	fstpt -12(%ebp)
	leal -4(%ebp),%ecx
	movw -4(%ebp),%dx
	andl $32767,%edx
	jne L25
	fldz
	fucompp
	fnstsw %ax
	andb $68,%ah
	xorb $64,%ah
	jne L21
	movl $0,(%ebx)
	fldz
	jmp L24
	.align 2,0x90
	.align 2,0x90
L21:
	fldt -12(%ebp)
	fadd %st(0),%st
	fstpt -12(%ebp)
	decl %edx
	movw (%ecx),%si
	andl $32767,%esi
	jne L22
	cmpl $-66,%edx
	jg L21
L22:
	addl %esi,%edx
	jmp L19
	.align 2,0x90
L25:
	fstp %st(0)
L19:
	addl $-16382,%edx
	movl %edx,(%ebx)
	movw (%ecx),%ax
	andl $-32768,%eax
	orl $16382,%eax
	movw %ax,(%ecx)
	fldt -12(%ebp)
L24:
	leal -32(%ebp),%esp
	popl %ebx
	popl %esi
	leave
	ret
