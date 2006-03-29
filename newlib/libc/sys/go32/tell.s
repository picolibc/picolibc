# /* This is file TELL.S */
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
	.globl _tell
_tell:
	pushl	%ebx
	pushl	%esi
	pushl	%edi
	movl	16(%esp),%ebx
	movl	$0,%ecx
	movl	$0,%edx
	movw	$0x4201,%ax
	int	$0x21
	popl	%edi
	popl	%esi
	popl	%ebx
	jb	syscall_error
	shll	$16,%edx
	andl	$0xffff,%eax
	orl	%edx,%eax
	ret
