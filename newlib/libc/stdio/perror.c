/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
FUNCTION
<<perror>>---print an error message on standard error

INDEX
	perror
INDEX
	_perror_r

SYNOPSIS
	#include <stdio.h>
	void perror(char *<[prefix]>);

	void _perror_r(struct _reent *<[reent]>, char *<[prefix]>);

DESCRIPTION
Use <<perror>> to print (on standard error) an error message
corresponding to the current value of the global variable <<errno>>.
Unless you use <<NULL>> as the value of the argument <[prefix]>, the
error message will begin with the string at <[prefix]>, followed by a
colon and a space (<<: >>). The remainder of the error message is one
of the strings described for <<strerror>>.

The alternate function <<_perror_r>> is a reentrant version.  The
extra argument <[reent]> is a pointer to a reentrancy structure.

RETURNS
<<perror>> returns no result.

PORTABILITY
ANSI C requires <<perror>>, but the strings issued vary from one
implementation to another.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include <string.h>
#include <sys/uio.h>
#include "local.h"

#define ADD(str) \
{ \
  v->iov_base = (void *)(str); \
  v->iov_len = strlen (v->iov_base); \
  v ++; \
  iov_cnt ++; \
}

void
_perror_r (struct _reent *ptr,
       const char *s)
{
  char *error;
  int dummy;
  struct iovec iov[4];
  struct iovec *v = iov;
  int iov_cnt = 0;
  FILE *fp = _stderr_r (ptr);

  CHECK_INIT (ptr, fp);
  if (s != NULL && *s != '\0')
    {
      ADD (s);
      ADD (": ");
    }

  if ((error = _strerror_r (ptr, ptr->_errno, 1, &dummy)) != NULL)
    ADD (error);

#ifdef __SCLE
  ADD ((fp->_flags & __SCLE) ? "\r\n" : "\n");
#else
  ADD ("\n");
#endif

  _newlib_flockfile_start (fp);
  fflush (fp);
  writev (fileno (fp), iov, iov_cnt);
  _newlib_flockfile_end (fp);
}

#ifndef _REENT_ONLY

void
perror (const char *s)
{
  _perror_r (_REENT, s);
}

#endif
