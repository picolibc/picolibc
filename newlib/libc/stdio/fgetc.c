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
<<fgetc>>, <<fgetc_unlocked>>---get a character from a file or stream

INDEX
	fgetc
INDEX
	fgetc_unlocked
INDEX
	_fgetc_r
INDEX
	_fgetc_unlocked_r

SYNOPSIS
	#include <stdio.h>
	int fgetc(FILE *<[fp]>);

	#define _BSD_SOURCE
	#include <stdio.h>
	int fgetc_unlocked(FILE *<[fp]>);

	#include <stdio.h>
	int _fgetc_r(struct _reent *<[ptr]>, FILE *<[fp]>);

	#define _BSD_SOURCE
	#include <stdio.h>
	int _fgetc_unlocked_r(struct _reent *<[ptr]>, FILE *<[fp]>);

DESCRIPTION
Use <<fgetc>> to get the next single character from the file or stream
identified by <[fp]>.  As a side effect, <<fgetc>> advances the file's
current position indicator.

For a macro version of this function, see <<getc>>.

<<fgetc_unlocked>> is a non-thread-safe version of <<fgetc>>.
<<fgetc_unlocked>> may only safely be used within a scope
protected by flockfile() (or ftrylockfile()) and funlockfile().  This
function may safely be used in a multi-threaded program if and only
if they are called while the invoking thread owns the (FILE *)
object, as is the case after a successful call to the flockfile() or
ftrylockfile() functions.  If threads are disabled, then
<<fgetc_unlocked>> is equivalent to <<fgetc>>.

The functions <<_fgetc_r>> and <<_fgetc_unlocked_r>> are simply reentrant
versions that are passed the additional reentrant structure pointer
argument: <[ptr]>.

RETURNS
The next character (read as an <<unsigned char>>, and cast to
<<int>>), unless there is no more data, or the host system reports a
read error; in either of these situations, <<fgetc>> returns <<EOF>>.

You can distinguish the two situations that cause an <<EOF>> result by
using the <<ferror>> and <<feof>> functions.

PORTABILITY
ANSI C requires <<fgetc>>.

<<fgetc_unlocked>> is a BSD extension also provided by GNU libc.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#include <_ansi.h>
#include <stdio.h>
#include "local.h"

int
_fgetc_r (struct _reent * ptr,
       FILE * fp)
{
  int result;
  CHECK_INIT(ptr, fp);
  _newlib_flockfile_start (fp);
  result = __sgetc_r (ptr, fp);
  _newlib_flockfile_end (fp);
  return result;
}

#ifndef _REENT_ONLY

int
fgetc (FILE * fp)
{
#if !defined(PREFER_SIZE_OVER_SPEED) && !defined(__OPTIMIZE_SIZE__)
  int result;
  struct _reent *reent = _REENT;

  CHECK_INIT(reent, fp);
  _newlib_flockfile_start (fp);
  result = __sgetc_r (reent, fp);
  _newlib_flockfile_end (fp);
  return result;
#else
  return _fgetc_r (_REENT, fp);
#endif
}

#endif /* !_REENT_ONLY */

