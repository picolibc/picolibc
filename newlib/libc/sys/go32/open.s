# /* This is file OPEN.S */
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

	.data
	.globl	__fmode
__fmode:
	.long	0x4000

	.text
	.globl _open
_open:
	testl	$0xc000,8(%esp)
	jnz	L0
	andl	$0xc000,__fmode
	movl	__fmode,%eax
	orl	%eax,8(%esp)
L0:
	movb	$2,%al
	jmp	turbo_assist

