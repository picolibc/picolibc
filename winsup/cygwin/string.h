/* string.h: Extra string defs

   Copyright 2001 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGWIN_STRING_H
#define _CYGWIN_STRING_H

#include_next <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#undef strchr
#define strchr cygwin_strchr
static inline __stdcall char *
strchr (const char *s, int c)
{
  register char * res;
  __asm__ __volatile__ ("\
	movb	%%al,%%ah\n\
1:	movb	(%1),%%al\n\
	cmpb	%%ah,%%al\n\
	je	2f\n\
	incl	%1\n\
	testb	%%al,%%al\n\
	jne	1b\n\
	xorl	%1,%1\n\
2:	movl	%1,%0\n\
	":"=a" (res), "=r" (s)
	:"0" (c), "1" (s));
  return res;
}

extern const char isalpha_array[];

#undef strcasematch
#define strcasematch cygwin_strcasematch

static inline int
cygwin_strcasematch (const char *cs, const char *ct)
{
  register int __res;
  int d0, d1;
  __asm__ ("\
	.global	_isalpha_array			\n\
	cld					\n\
	andl	$0xff,%%eax			\n\
1:	lodsb					\n\
	scasb					\n\
	je	2f				\n\
	xorb	_isalpha_array(%%eax),%%al	\n\
	cmpb	-1(%%edi),%%al			\n\
	jne	3f				\n\
2:	testb	%%al,%%al			\n\
	jnz	1b				\n\
	movl	$1,%%eax			\n\
	jmp	4f				\n\
3:	xor	%0,%0				\n\
4:"
        :"=a" (__res), "=&S" (d0), "=&D" (d1)
		     : "1" (cs),   "2" (ct));

  return __res;
}

#undef strncasematch
#define strncasematch cygwin_strncasematch

static inline int
cygwin_strncasematch (const char *cs, const char *ct, size_t n)
{
  register int __res;
  int d0, d1, d2;
  __asm__ ("\
	.global	_isalpha_array;			\n\
	cld					\n\
	andl	$0xff,%%eax			\n\
1:	decl	%3				\n\
	js	3f				\n\
	lodsb					\n\
	scasb					\n\
	je	2f				\n\
	xorb	_isalpha_array(%%eax),%%al	\n\
	cmpb	-1(%%edi),%%al			\n\
	jne	4f				\n\
2:	testb	%%al,%%al			\n\
	jnz	1b				\n\
3:	movl	$1,%%eax			\n\
	jmp	5f				\n\
4:	xor	%0,%0				\n\
5:"
	:"=a" (__res), "=&S" (d0), "=&D" (d1), "=&c" (d2)
		       :"1" (cs),  "2" (ct), "3" (n));

  return __res;
}

#ifdef __cplusplus
}
#endif
#endif /* _CYGWIN_STRING_H */
