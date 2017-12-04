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
<<ferror>>, <<ferror_unlocked>>---test whether read/write error has occurred

INDEX
	ferror
INDEX
	ferror_unlocked

SYNOPSIS
	#include <stdio.h>
	int ferror(FILE *<[fp]>);

	#define _BSD_SOURCE
	#include <stdio.h>
	int ferror_unlocked(FILE *<[fp]>);

DESCRIPTION
The <<stdio>> functions maintain an error indicator with each file
pointer <[fp]>, to record whether any read or write errors have
occurred on the associated file or stream.
Use <<ferror>> to query this indicator.

See <<clearerr>> to reset the error indicator.

<<ferror_unlocked>> is a non-thread-safe version of <<ferror>>.
<<ferror_unlocked>> may only safely be used within a scope
protected by flockfile() (or ftrylockfile()) and funlockfile().  This
function may safely be used in a multi-threaded program if and only
if they are called while the invoking thread owns the (FILE *)
object, as is the case after a successful call to the flockfile() or
ftrylockfile() functions.  If threads are disabled, then
<<ferror_unlocked>> is equivalent to <<ferror>>.

RETURNS
<<ferror>> returns <<0>> if no errors have occurred; it returns a
nonzero value otherwise.

PORTABILITY
ANSI C requires <<ferror>>.

<<ferror_unlocked>> is a BSD extension also provided by GNU libc.

No supporting OS subroutines are required.
*/

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "%W% (Berkeley) %G%";
#endif /* LIBC_SCCS and not lint */

#include <_ansi.h>
#include <stdio.h>
#include "local.h"

/* A subroutine version of the macro ferror.  */

#undef ferror

int
ferror (FILE * fp)
{
  int result;
  CHECK_INIT(_REENT, fp);
  _newlib_flockfile_start (fp);
  result = __sferror (fp);
  _newlib_flockfile_end (fp);
  return result;
}
