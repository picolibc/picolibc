# /* This is file GETWD.S */
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
	.globl _getwd
_getwd:
	pushl	%ebx
	pushl	%esi
	pushl	%edi
	movl	16(%esp),%esi
	movb	$47,(%esi)
	incl	%esi
	movb	$0,%dl
	movb	$0x47,%ah
	int	$0x21
	movl	16(%esp),%eax
	popl	%edi
	popl	%esi
	popl	%ebx
	ret
