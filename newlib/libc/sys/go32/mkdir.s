# /* This is file MKDIR.S */
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
	.globl _mkdir
_mkdir:
	pushl	%ebx
	pushl	%esi
	pushl	%edi
	movl	16(%esp),%edx
	movb	$0x39,%ah
	int	$0x21
	popl	%edi
	popl	%esi
	popl	%ebx
	jmp	syscall_check
