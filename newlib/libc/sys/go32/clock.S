/* This is file CLOCK.S */
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
**
** Modified by C. Sandmann for DPMI support (sandmann@clio.rice.edu)
*/
	.data
clock_init:
	.long	0

	.text
	.globl _clock
_clock:
	call	_rawclock
	imull	$54931,%eax,%eax
	ret
	
	.align  4,0x90
	.globl	_rawclock
_rawclock:
	movzwl	__core_select,%eax
	testl	%eax,%eax
	je	old_clock	/* Image run with pre-DPMI extender */
	movw	%ax,%gs
	movl	%gs:0x46c(,1),%eax
	jmp	check_initted
old_clock:
	movl	0xe000046c,%eax
check_initted:
	cmp	$0,clock_init
	jne	clock_initted
	movl	%eax,clock_init
clock_initted:
	subl	clock_init,%eax
	ret

