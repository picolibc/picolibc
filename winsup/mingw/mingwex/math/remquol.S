/*
 * Written by Ulrich Drepper <drepper@cygnus.com>.
 * Based on e_remainder by J.T. Conklin <jtc@netbsd.org>.
 * Removed header file dependency for use in libmingwex.a by
 *   Danny Smith <dannysmith@users.sourceforge.net>
 * Public domain.
 */
        .text
	.align 4;
.globl _remquol;
 _remquol:
        fldt 4 +12(%esp)
        fldt 4(%esp)
1:	fprem1
        fstsw %ax
        sahf
        jp 1b
        fstp %st(1)
        movl %eax, %ecx
        shrl $8, %eax
        shrl $12, %ecx
        andl $4, %ecx
        andl $3, %eax
        orl %eax, %ecx
        movl $0xef2960, %eax
        shrl %cl, %eax
        andl $3, %eax
        movl 4 +12 +12(%esp), %ecx      
        movl 4 +8(%esp), %edx
        xorl 4 +12 +8(%esp), %edx
        testl $0x8000, %edx
        jz 1f
        negl %eax
1:	movl %eax, (%ecx)
      
        ret
