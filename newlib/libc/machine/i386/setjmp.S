/* This is file is a merger of SETJMP.S and LONGJMP.S */
/*
 *  This file was modified to use the __USER_LABEL_PREFIX__ and
 *  __REGISTER_PREFIX__ macros defined by later versions of GNU cpp by
 *  Joel Sherrill (joel@OARcorp.com)
 *
 ** Copyright (C) 1991 DJ Delorie, 24 Kirsten Ave, Rochester NH 03867-2954
 **
 ** This file is distributed under the terms listed in the document
 ** "copying.dj", available from DJ Delorie at the address above.
 ** A copy of "copying.dj" should accompany this file; if not, a copy
 ** should be available from where this file was obtained.  This file
 ** may not be distributed without a verbatim copy of "copying.dj".
 **
 ** This file is distributed WITHOUT ANY WARRANTY; without even the implied
 ** warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

 /*
 **	jmp_buf:
 **	 eax ebx ecx edx esi edi ebp esp eip
 **	 0   4   8   12  16  20  24  28  32
 */

/* These are predefined by new versions of GNU cpp.  */
 
#ifndef __USER_LABEL_PREFIX__
#define __USER_LABEL_PREFIX__ _
#endif
 
/***************************** TEMPORARY ******************************
 * 
 *  It appears that the i386-coff configuration of gcc 2.6.2 does not
 *  do the __REGISTER_PREFIX__ correctly.  Fix this when it does.
 **********************************************************************/
#define __REGISTER_PREFIX__ %
#ifndef __REGISTER_PREFIX__
#define __REGISTER_PREFIX__ 
#endif
 
/* ANSI concatenation macros.  */
 
#define CONCAT1(a, b) CONCAT2(a, b)
#define CONCAT2(a, b) a ## b
 
/* Use the right prefix for global labels.  */
 
#define SYM(x) CONCAT1 (__USER_LABEL_PREFIX__, x)
 
/* Use the right prefix for registers.  */
 
#define REG(x) CONCAT1 (__REGISTER_PREFIX__, x)
 
#define eax REG (eax)
#define ebx REG (ebx)
#define ecx REG (ecx)
#define edx REG (edx)
#define esi REG (esi)
#define edi REG (edi)
#define ebp REG (ebp)
#define esp REG (esp)
 
#define ax REG (ax)
#define bx REG (bx)
#define cx REG (cx)
#define dx REG (dx)

#define ah REG (ah)
#define bh REG (bh)
#define ch REG (ch)
#define dh REG (dh)

#define al REG (al)
#define bl REG (bl)
#define cl REG (cl)
#define dl REG (dl)

        .global SYM (setjmp)
        .global SYM (longjmp)
 
SYM (setjmp):

	pushl	ebp
	movl	esp,ebp

	pushl	edi
	movl	8 (ebp),edi

	movl	eax,0 (edi)
	movl	ebx,4 (edi)
	movl	ecx,8 (edi)
	movl	edx,12 (edi)
	movl	esi,16 (edi)

	movl	-4 (ebp),eax
	movl	eax,20 (edi)

	movl	0 (ebp),eax
	movl	eax,24 (edi)

	movl	esp,eax
	addl	$12,eax
	movl	eax,28 (edi)
	
	movl	4 (ebp),eax
	movl	eax,32 (edi)

	popl	edi
	movl	$0,eax
	leave
	ret

SYM (longjmp):
	pushl	ebp
	movl	esp,ebp

	movl	8(ebp),edi	/* get jmp_buf */
	movl	12(ebp),eax	/* store retval in j->eax */
	movl	eax,0(edi)

	movl	24(edi),ebp

	cli
	movl	28(edi),esp
	
	pushl	32(edi)	

	movl	0(edi),eax
	movl	4(edi),ebx
	movl	8(edi),ecx
	movl	12(edi),edx
	movl	16(edi),esi
	movl	20(edi),edi
	sti

	ret
