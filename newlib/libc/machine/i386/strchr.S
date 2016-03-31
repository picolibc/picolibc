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

	.global SYM (strchr)
       SOTYPE_FUNCTION(strchr)

SYM (strchr):

#ifdef __iamcu__
	xorl ecx,ecx
	movb dl,cl

/* loop while (*s && *s++ != c) */
	leal -1(eax),eax
L15:
	incl eax
	movb (eax),dl
	testb dl,dl
	je L14
	cmpb cl,dl
	jne L15

L14:
/*  if (*s == c) return address otherwise return NULL */
	cmpb cl,(eax)
	je L19
	xorl eax,eax

L19:
	ret
#else
	pushl ebp
	movl esp,ebp
	pushl edi
	pushl ebx
	xorl ebx,ebx
	movl 8(ebp),edi
	addb 12(ebp),bl

#ifndef __OPTIMIZE_SIZE__
/* Special case strchr(p,0).  */
	je L25

/* Do byte-wise checks until string is aligned.  */
	test $3,edi
	je L5
	movl edi,eax
	movb (eax),cl
	testb cl,cl
	je L14
	cmpb bl,cl
	je L19
	incl edi

	test $3,edi
	je L5
	movl edi,eax
	movb (eax),cl
	testb cl,cl
	je L14
	cmpb bl,cl
	je L19
	incl edi

	test $3,edi
	je L5
	movl edi,eax
	movb (eax),cl
	testb cl,cl
	je L14
	cmpb bl,cl
	je L19
	incl edi

/* create 4 byte mask which is just the desired byte repeated 4 times */
L5:
	movl ebx,ecx
	sall $8,ebx
	subl $4,edi
	orl ecx,ebx
	movl ebx,edx
	sall $16,ebx
	orl edx,ebx

/* loop performing 4 byte mask checking for 0 byte or desired byte */
	.p2align 4,,7
L10:
	addl $4,edi
	movl (edi),ecx
	leal -16843009(ecx),edx
	movl ecx,eax
	notl eax
	andl eax,edx
	testl $-2139062144,edx
	jne L9

	xorl ebx,ecx
	leal -16843009(ecx),edx
	notl ecx
	andl ecx,edx
	testl $-2139062144,edx
	je L10
#endif /* not __OPTIMIZE_SIZE__ */

/* loop while (*s && *s++ != c) */
L9:
	leal -1(edi),eax
	.p2align 4,,7
L15:
	incl eax
	movb (eax),dl
	testb dl,dl
	je L14
	cmpb bl,dl
	jne L15

L14:
/*  if (*s == c) return address otherwise return NULL */
	cmpb bl,(eax)
	je L19
	xorl eax,eax

L19:
	leal -8(ebp),esp
	popl ebx
	popl edi
	leave
	ret

#ifndef __OPTIMIZE_SIZE__
/* Special case strchr(p,0).  */
#if 0
	/* Hideous performance on modern machines.  */
L25:
	cld
	movl $-1,ecx
	xor eax,eax
	repnz
	scasb
	leal -1(edi),eax
	jmp L19
#endif
L25:
/* Do byte-wise checks until string is aligned.  */
	test $3,edi
	je L26
	movl edi,eax
	movb (eax),cl
	testb cl,cl
	je L19
	incl edi

	test $3,edi
	je L26
	movl edi,eax
	movb (eax),cl
	testb cl,cl
	je L19
	incl edi

	test $3,edi
	je L26
	movl edi,eax
	movb (eax),cl
	testb cl,cl
	je L19
	incl edi

L26:
	subl $4,edi

/* loop performing 4 byte mask checking for desired 0 byte */
	.p2align 4,,7
L27:
	addl $4,edi
	movl (edi),ecx
	leal -16843009(ecx),edx
	movl ecx,eax
	notl eax
	andl eax,edx
	testl $-2139062144,edx
	je L27

	jmp L9

#endif /* !__OPTIMIZE_SIZE__ */

#endif /* __iamcu__ */
