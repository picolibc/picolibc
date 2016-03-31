/* This is file is a merger of SETJMP.S and LONGJMP.S */
/*
 *  This file was modified to use the __USER_LABEL_PREFIX__ and
 *  __REGISTER_PREFIX__ macros defined by later versions of GNU cpp by
 *  Joel Sherrill (joel@OARcorp.com)
 *  Slight change: now includes i386mach.h for this (Werner Almesberger)
 *
 * Copyright (C) 1991 DJ Delorie
 * All rights reserved.
 *
 * Redistribution, modification, and use in source and binary forms is permitted
 * provided that the above copyright notice and following paragraph are
 * duplicated in all such forms.
 *
 * This file is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

 /*
 **	jmp_buf:
 **	 eax ebx ecx edx esi edi ebp esp eip
 **	 0   4   8   12  16  20  24  28  32
 **
 **	Intel MCU jmp_buf:
 **	 ebx esi edi ebp esp eip
 **	 0   4   8   12  16  20
 */

       #include "i386mach.h"

        .global SYM (setjmp)
        .global SYM (longjmp)
       SOTYPE_FUNCTION(setjmp)
       SOTYPE_FUNCTION(longjmp)
 
SYM (setjmp):

#ifdef __iamcu__
	/* Store EIP.  */
	movl	0(esp),ecx
	movl	ecx,20(eax)

	movl	ebx,0 (eax)
	movl	esi,4 (eax)
	movl	edi,8 (eax)
	movl	ebp,12(eax)

	/* Skip return address, which will be pushed onto stack in
	   longjmp, and store SP.  */
	leal	4(esp),ecx
	movl	ecx,16(eax)

	xorl	eax,eax
#else
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
#endif
	ret

SYM (longjmp):
#ifdef __iamcu__
	/* Check retval.  */
	testl	edx,edx
	jne	0f
	incl	edx
0:
	/* Restore stack first.  */
	movl	16(eax),esp

	/* Put return address on stack.  */
	pushl	20(eax)

	movl	0(eax),ebx
	movl	4(eax),esi
	movl	8(eax),edi
	movl	12(eax),ebp
	movl	edx,eax
#else
	pushl	ebp
	movl	esp,ebp

	movl	8(ebp),edi	/* get jmp_buf */
	movl	12(ebp),eax	/* store retval in j->eax */
	testl	eax,eax
	jne	0f
	incl	eax
0:
	movl	eax,0(edi)

	movl	24(edi),ebp

       __CLI
	movl	28(edi),esp
	
	pushl	32(edi)	

	movl	0(edi),eax
	movl	4(edi),ebx
	movl	8(edi),ecx
	movl	12(edi),edx
	movl	16(edi),esi
	movl	20(edi),edi
       __STI
#endif

	ret
