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

#ifdef __cplusplus
}
#endif
#endif /* _CYGWIN_STRING_H */
