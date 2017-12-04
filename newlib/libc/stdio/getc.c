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
<<getc>>---read a character (macro)

INDEX
	getc
INDEX
	_getc_r

SYNOPSIS
	#include <stdio.h>
	int getc(FILE *<[fp]>);

	#include <stdio.h>
	int _getc_r(struct _reent *<[ptr]>, FILE *<[fp]>);

DESCRIPTION
<<getc>> is a macro, defined in <<stdio.h>>.  You can use <<getc>>
to get the next single character from the file or stream
identified by <[fp]>.  As a side effect, <<getc>> advances the file's
current position indicator.

For a subroutine version of this macro, see <<fgetc>>.

The <<_getc_r>> function is simply the reentrant version of <<getc>>
which passes an additional reentrancy structure pointer argument: <[ptr]>.

RETURNS
The next character (read as an <<unsigned char>>, and cast to
<<int>>), unless there is no more data, or the host system reports a
read error; in either of these situations, <<getc>> returns <<EOF>>.

You can distinguish the two situations that cause an <<EOF>> result by
using the <<ferror>> and <<feof>> functions.

PORTABILITY
ANSI C requires <<getc>>; it suggests, but does not require, that
<<getc>> be implemented as a macro.  The standard explicitly permits
macro implementations of <<getc>> to use the argument more than once;
therefore, in a portable program, you should not use an expression
with side effects as the <<getc>> argument.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "%W% (Berkeley) %G%";
#endif /* LIBC_SCCS and not lint */

#include <_ansi.h>
#include <stdio.h>
#include "local.h"

/*
 * A subroutine version of the macro getc.
 */

#undef getc

int
_getc_r (struct _reent *ptr,
       register FILE *fp)
{
  int result;
  CHECK_INIT (ptr, fp);
  _newlib_flockfile_start (fp);
  result = __sgetc_r (ptr, fp);
  _newlib_flockfile_end (fp);
  return result;
}

#ifndef _REENT_ONLY

int
getc (register FILE *fp)
{
  int result;
  struct _reent *reent = _REENT;

  CHECK_INIT (reent, fp);
  _newlib_flockfile_start (fp);
  result = __sgetc_r (reent, fp);
  _newlib_flockfile_end (fp);
  return result;
}

#endif /* !_REENT_ONLY */
