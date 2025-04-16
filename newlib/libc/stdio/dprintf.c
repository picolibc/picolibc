/* Copyright 2005, 2007 Shaun Jackman
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

/*
FUNCTION
<<dprintf>>, <<vdprintf>>---print to a file descriptor

INDEX
	dprintf
INDEX
	_dprintf_r
INDEX
	vdprintf
INDEX
	_vdprintf_r

SYNOPSIS
	#include <stdio.h>
	#include <stdarg.h>
	int dprintf(int <[fd]>, const char *restrict <[format]>, ...);
	int vdprintf(int <[fd]>, const char *restrict <[format]>,
			va_list <[ap]>);
	int dprintf( int <[fd]>,
			const char *restrict <[format]>, ...);
	int vdprintf( int <[fd]>,
			const char *restrict <[format]>, va_list <[ap]>);

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

#define _DEFAULT_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include "local.h"

int
dprintf (int fd,
       const char *__restrict format, ...)
{
  va_list ap;
  int n;

  va_start (ap, format);
  n = vdprintf ( fd, format, ap);
  va_end (ap);
  return n;
}

__nano_reference(dprintf, diprintf);
