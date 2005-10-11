/* Copyright 2005 Shaun Jackman
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef _HAVE_STDC
#include <stdarg.h>
#else
#include <varargs.h>
#endif

int
_DEFUN (_vdprintf_r, (ptr, fd, format, ap),
	struct _reent *ptr _AND
	int fd _AND
	_CONST char *format _AND
	va_list ap)
{
	char *p;
	int n;
	_REENT_SMALL_CHECK_INIT (ptr);
	n = _vasprintf_r (ptr, &p, format, ap);
	if (n == -1) return -1;
	n = _write_r (ptr, fd, p, n);
	_free_r (ptr, p);
	return n;
}

#ifndef _REENT_ONLY

int 
_DEFUN (vdprintf, (fd, format, ap),
	int fd _AND
	_CONST char *format _AND
	va_list ap)
{
	_REENT_SMALL_CHECK_INIT (_REENT);
	return _vdprintf_r (_REENT, fd, format, ap);
}

#endif /* ! _REENT_ONLY */
