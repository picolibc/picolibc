# /* This is file GERRNO.S */
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

# Modified to use newlib's reent mechanism, 960414, dje.
# Reentrancy isn't really supported of course, the purpose is to
# record `errno' in the normal place.

	.text
	.globl	syscall_error
syscall_error:
	pushl	%eax
	call	___errno
	popl	%edx
	mov	%edx,(%eax)
	mov	$-1,%eax
	ret

	.globl	syscall_check
syscall_check:
	jb	syscall_error
	mov	$0,%eax
	ret
