/*
 * ====================================================
 * Copyright (C) 1998, 2002, 2008 by Red Hat Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

	#include "i386mach.h"

	.global SYM (strlen)
       SOTYPE_FUNCTION(strlen)

SYM (strlen):

	pushl ebp
	movl esp,ebp
	pushl edi
	movl 8(ebp),edx

#ifdef __OPTIMIZE_SIZE__
	cld
	movl edx,edi
	movl $4294967295,ecx
	xor eax,eax
	repnz
	scasb
#else
/* Modern x86 hardware is much faster at double-word
   manipulation than with bytewise repnz scasb.  */

/* Do byte-wise checks until string is aligned.  */
	movl edx,edi
	test $3,edi
	je L5
	movb (edi),cl
	incl edi
	testb cl,cl
	je L15

	test $3,edi
	je L5
	movb (edi),cl
	incl edi
	testb cl,cl
	je L15

	test $3,edi
	je L5
	movb (edi),cl
	incl edi
	testb cl,cl
	je L15

L5:
	subl $4,edi

/* loop performing 4 byte mask checking for desired 0 byte */
	.p2align 4,,7
L10:
	addl $4,edi
	movl (edi),ecx
	leal -16843009(ecx),eax
	notl ecx
	andl ecx,eax
	testl $-2139062144,eax
	je L10

/* Find which of four bytes is 0.  */
	notl ecx
	incl edi

	testb cl,cl
	je L15
	incl edi
	shrl $8,ecx

	testb cl,cl
	je L15
	incl edi
	shrl $8,ecx

	testb cl,cl
	je L15
	incl edi

#endif

L15:
	subl edx,edi
	leal -1(edi),eax

	leal -4(ebp),esp
	popl edi
	leave
	ret
