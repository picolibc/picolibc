/* This is file KBHIT.S */
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
*/

	.globl	_kbhit
_kbhit:
	pushl	%ebp
	movl	%esp,%ebp
	pushl	%ebx
	pushl	%esi
	pushl	%edi

try_again:
	movl	$0x41a,%eax
	call	dosmemsetup
	movzwl	%gs:(%eax),%ebx
	movzwl	%gs:2(%eax),%ecx
	cmp	%ebx,%ecx
	je	key_not_hit

	movb	$0x11,%ah
	int	$0x16
	jz	key_not_hit
	cmp	$0,%eax
	jne	key_hit
	movb	$0x10,%ah
	int	$0x16
	jmp	try_again

key_not_hit:
	movl	$0,%eax
	jmp	L1

key_hit:
	movl	$1,%eax
L1:
	popl	%edi
	popl	%esi
	popl	%ebx
	leave
	ret
