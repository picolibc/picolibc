# /* This is file RENAME.S */
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
	.globl _rename
_rename:
	pushl	%ebx
	pushl	%esi
	pushl	%edi
	movl	20(%esp),%edx
	movb	$0x41,%ah
	int	$0x21		# remove "to" 
	movl	16(%esp),%edx
	movl	20(%esp),%edi
	movb	$0x56,%ah
	int	$0x21		#  move "from" to "to" 
	popl	%edi
	popl	%esi
	popl	%ebx
	jmp	syscall_check
