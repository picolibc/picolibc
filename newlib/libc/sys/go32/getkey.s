/* This is file GETKEY.S */
/*
** Copyright (C) 1993 DJ Delorie
**
** This file is distributed under the terms listed in the document
** "copying.dj".
** A copy of "copying.dj" should accompany this file; if not, a copy
** should be available from where this file was obtained.  This file
** may not be distributed without a verbatim copy of "copying.dj".
**
** This file is distributed WITHOUT ANY WARRANTY; without even the implied
** warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

Modified by J. Alan Eldridge, Liberty Brokerage, 77 Water St, NYC 10005

Changed handling of characters starting with 0xE0:
	Now calls interrupt 16, function 10
	if leading byte was 0x00, ah = 0x01
	if leading byte was 0xE0, ah = 0x02

	The main function is now called getxkey()...
	getkey is provided to maintain compatibility with
	already written software
*/

	.globl	_getxkey
_getxkey:
	pushl	%ebx
	pushl	%esi
	pushl	%edi
	movb	$0x10,%ah
	int	$0x16
	mov	$1,%bl
	cmpb	$0,%al
	je	L0
	cmpb	$0xE0,%al
	jne	L1
	inc	%bl
L0:
	movb	%ah,%al
	movb	%bl,%ah
	jmp	L2
L1:
	movb	$0,%ah
L2:
	andl	$0xffff,%eax
	popl	%edi
	popl	%esi
	popl	%ebx
	ret

	.globl	_getkey
_getkey:
	call	_getxkey
	testb	$0x02,%ah
	jz	L3
	movb	$0x01,%ah
L3:
	ret
