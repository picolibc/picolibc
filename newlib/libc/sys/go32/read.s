# /* This is file READ.S */
# /*
# ** Copyright (C) 1991 DJ Delorie
# **
# ** This file is distributed under the terms listed in the document
# ** "copying.dj".
# ** A copy of "copying.dj" should accompany this file; if not, a copy
# ** should be available from where this file was obtained.  This file
# ** may not be distributed without a verbatim copy of "copying.dj".
# **
# ** This file is distributed WITHOUT ANY WARRANTY; without even the implied
# ** warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# */

	.text
	.globl _read
_read:
	pushl	%ebx
	pushl	%esi
	pushl	%edi
	movl	16(%esp),%ebx
	movl	20(%esp),%edx
	movl	24(%esp),%ecx
	movb	$0x3f,%ah
	int	$0x21
	popl	%edi
	popl	%esi
	popl	%ebx
	jb	syscall_error
	ret
