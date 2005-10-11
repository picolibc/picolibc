/* Copyright 2005 Shaun Jackman
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

/*
FUNCTION
<<dprintf>>, <<vdprintf>>---print to a file descriptor

INDEX
	dprintf
INDEX
	vdprintf

ANSI_SYNOPSIS
	#include <stdio.h>
	#include <stdarg.h>
	int dprintf(int <[fd]>, const char *<[format]>, ...);
	int vdprintf(int <[fd]>, const char *<[format]>, va_list <[ap]>);
	int _dprintf_r(struct _reent *<[ptr]>, int <[fd]>, 
			const char *<[format]>, ...);
	int _vdprintf_r(struct _reent *<[ptr]>, int <[fd]>, 
			const char *<[format]>, va_list <[ap]>);

TRAD_SYNOPSIS
        #include <stdio.h>
        #include <varargs.h>

        int dprintf(<[fd]>, <[format]> [, <[arg]>, ...])
	int <[fd]>;
        char *<[format]>;

        int vdprintf(<[fd]>, <[fmt]>, <[list]>)
	int <[fd]>;
        char *<[fmt]>;
        va_list <[list]>;

        int _dprintf_r(<[ptr]>, <[fd]>, <[format]> [, <[arg]>, ...])
	struct _reent *<[ptr]>;
	int <[fd]>;
        char *<[format]>;

        int _vdprintf_r(<[ptr]>, <[fd]>, <[fmt]>, <[list]>)
	struct _reent *<[ptr]>;
	int <[fd]>;
        char *<[fmt]>;
        va_list <[list]>;

DESCRIPTION
<<dprintf>> and <<vdprintf>> allow printing a format, similarly to
<<printf>>, but write to a file descriptor instead of to a <<FILE>>
stream.

The functions <<_dprintf_r>> and <<_vdprintf_r>> are simply
reentrant versions of the functions above.

RETURNS
The return value and errors are exactly as for <<write>>, except that
<<errno>> may also be set to <<ENOMEM>> if the heap is exhausted.

PORTABILITY
This function is originally a GNU extension in glibc and is not portable.

Supporting OS subroutines required: <<sbrk>>, <<write>>.
*/

#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include <unistd.h>
#ifdef _HAVE_STDC
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#ifdef _HAVE_STDC
int
_dprintf_r(struct _reent *ptr, int fd, _CONST char *format, ...)
#else
int
_dprintf_r(ptr, fd, format, va_alist)
	struct _reent *ptr;
	int fd;
	char *format;
	va_dcl
#endif
{
	va_list ap;
	int n;
	_REENT_SMALL_CHECK_INIT (ptr);
#ifdef _HAVE_STDC
  	va_start (ap, format);
#else
  	va_start (ap);
#endif
	n = _vdprintf_r (ptr, fd, format, ap);
	va_end (ap);
	return n;
}

#ifndef _REENT_ONLY

#ifdef _HAVE_STDC
int
dprintf(int fd, _CONST char *format, ...)
#else
int
dprintf(fd, format, va_alist)
	struct _reent *ptr;
	int fd;
	char *format;
	va_dcl
#endif
{
	va_list ap;
	int n;
	_REENT_SMALL_CHECK_INIT (_REENT);
#ifdef _HAVE_STDC
  	va_start (ap, format);
#else
  	va_start (ap);
#endif
	n = _vdprintf_r (_REENT, fd, format, ap);
	va_end (ap);
	return n;
}

#endif /* ! _REENT_ONLY */
