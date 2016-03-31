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

	.global	SYM (memchr)
       SOTYPE_FUNCTION(memchr)

SYM (memchr):
#ifdef __iamcu__
	pushl	edi
	movl	eax,edi
	movl	edx,eax
	xorl	edx,edx
	testl	ecx,ecx
	jz	L20

	repnz
	scasb

	setnz	dl
	decl	edi

	decl	edx
	andl	edi,edx
L20:
	movl	edx,eax

	popl	edi
#else
	pushl	ebp
	movl	esp,ebp
	pushl	edi
	movzbl	12(ebp),eax
	movl	16(ebp),ecx
	movl	8(ebp),edi
	xorl	edx,edx
	testl	ecx,ecx
	jz	L20

#ifdef __OPTIMIZE_SIZE__

	cld
	repnz
	scasb

	setnz	dl
	decl	edi

#else /* !__OPTIMIZE_SIZE__ */
/* Do byte-wise checks until string is aligned.  */
	testl	$3,edi
	je	L5
	cmpb	(edi),al
	je	L15
	incl	edi
	decl	ecx
	je	L20

	testl	$3,edi
	je	L5
	cmpb	(edi),al
	je	L15
	incl	edi
	decl	ecx
	je	L20

	testl	$3,edi
	je	L5
	cmpb	(edi),al
	je	L15
	incl	edi
	decl	ecx
	je	L20

/* Create a mask, then check a word at a time.  */
L5:
	movb	al,ah
	movl	eax,edx
	sall	$16,edx
	orl	edx,eax
	pushl	ebx

	.p2align 4,,7
L8:
	subl	$4,ecx
	jc	L9
	movl	(edi),edx
	addl	$4,edi
	xorl	eax,edx
	leal	-16843009(edx),ebx
	notl	edx
	andl	edx,ebx
	testl	$-2139062144,ebx
	je	L8

	subl	$4,edi

L9:
	popl	ebx
	xorl	edx,edx
	addl	$4,ecx
	je	L20

/* Final byte-wise checks.  */
	.p2align 4,,7
L10:
	cmpb	(edi),al
	je	L15
	incl	edi
	decl	ecx
	jne	L10

	xorl	edi,edi

#endif /* !__OPTIMIZE_SIZE__ */

L15:
	decl	edx
	andl	edi,edx
L20:
	movl	edx,eax

	leal	-4(ebp),esp
	popl	edi
	leave
#endif
	ret
