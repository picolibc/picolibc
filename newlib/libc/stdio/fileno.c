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
<<fileno>>, <<fileno_unlocked>>---return file descriptor associated with stream

INDEX
	fileno
INDEX
	fileno_unlocked

ANSI_SYNOPSIS
	#include <stdio.h>
	int fileno(FILE *<[fp]>);

	#define _BSD_SOURCE
	#include <stdio.h>
	int fileno_unlocked(FILE *<[fp]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	int fileno(<[fp]>)
	FILE *<[fp]>;

	#define _BSD_SOURCE
	#include <stdio.h>
	int fileno_unlocked(<[fp]>)
	FILE *<[fp]>;

DESCRIPTION
You can use <<fileno>> to return the file descriptor identified by <[fp]>.

<<fileno_unlocked>> is a non-thread-safe version of <<fileno>>.
<<fileno_unlocked>> may only safely be used within a scope
protected by flockfile() (or ftrylockfile()) and funlockfile().  This
function may safely be used in a multi-threaded program if and only
if they are called while the invoking thread owns the (FILE *)
object, as is the case after a successful call to the flockfile() or
ftrylockfile() functions.  If threads are disabled, then
<<fileno_unlocked>> is equivalent to <<fileno>>.

RETURNS
<<fileno>> returns a non-negative integer when successful.
If <[fp]> is not an open stream, <<fileno>> returns -1.

PORTABILITY
<<fileno>> is not part of ANSI C.
POSIX requires <<fileno>>.

<<fileno_unlocked>> is a BSD extension also provided by GNU libc.

Supporting OS subroutines required: none.
*/

#include <_ansi.h>
#include <stdio.h>
#include <errno.h>
#include "local.h"

int
_DEFUN(fileno, (f),
       FILE * f)
{
  int result;
  CHECK_INIT (_REENT, f);
  _newlib_flockfile_start (f);
  if (f->_flags)
    result = __sfileno (f);
  else
    {
      result = -1;
      _REENT->_errno = EBADF;
    }
  _newlib_flockfile_end (f);
  return result;
}
