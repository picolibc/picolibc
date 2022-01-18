/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * and/or other materials related to such
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
#include "local.h"

#define WRITE_STR(str) \
{ \
  const char *p = (str); \
  size_t len = strlen (p); \
  while (len) \
    { \
      ssize_t len1 = _write_r (ptr, fileno (fp), p, len); \
      if (len1 < 0) \
	break; \
      len -= len1; \
      p += len1; \
    } \
}

void
_perror_r (struct _reent *ptr,
       const char *s)
{
  char *error;
  int dummy;
  FILE *fp = _stderr_r (ptr);

  CHECK_INIT (ptr, fp);

  _newlib_flockfile_start(fp);
  _fflush_r (ptr, fp);
  if (s != NULL && *s != '\0')
    {
      WRITE_STR (s);
      WRITE_STR (": ");
    }

  if ((error = _strerror_r (ptr, _REENT_ERRNO(ptr), 1, &dummy)) != NULL)
    WRITE_STR (error);

#ifdef __SCLE
  WRITE_STR ((fp->_flags & __SCLE) ? "\r\n" : "\n");
#else
  WRITE_STR ("\n");
#endif
  fp->_flags &= ~__SOFF;
  _newlib_flockfile_end(fp);
}

#ifndef _REENT_ONLY

void
perror (const char *s)
{
  _perror_r (_REENT, s);
}

#endif
