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
<<feof>>, <<feof_unlocked>>---test for end of file

INDEX
	feof
INDEX
	feof_unlocked

ANSI_SYNOPSIS
	#include <stdio.h>
	int feof(FILE *<[fp]>);

	#define _BSD_SOURCE
	#include <stdio.h>
	int feof_unlocked(FILE *<[fp]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	int feof(<[fp]>)
	FILE *<[fp]>;

	#define _BSD_SOURCE
	#include <stdio.h>
	int feof_unlocked(<[fp]>)
	FILE *<[fp]>;

DESCRIPTION
<<feof>> tests whether or not the end of the file identified by <[fp]>
has been reached.

<<feof_unlocked>> is a non-thread-safe version of <<feof>>.
<<feof_unlocked>> may only safely be used within a scope
protected by flockfile() (or ftrylockfile()) and funlockfile().  This
function may safely be used in a multi-threaded program if and only
if they are called while the invoking thread owns the (FILE *)
object, as is the case after a successful call to the flockfile() or
ftrylockfile() functions.  If threads are disabled, then
<<feof_unlocked>> is equivalent to <<feof>>.

RETURNS
<<feof>> returns <<0>> if the end of file has not yet been reached; if
at end of file, the result is nonzero.

PORTABILITY
<<feof>> is required by ANSI C.

<<feof_unlocked>> is a BSD extension also provided by GNU libc.

No supporting OS subroutines are required.
*/

#include <stdio.h>
#include "local.h"

/* A subroutine version of the macro feof.  */

#undef feof

int 
_DEFUN(feof, (fp),
       FILE * fp)
{
  int result;
  CHECK_INIT(_REENT, fp);
  _newlib_flockfile_start (fp);
  result = __sfeof (fp);
  _newlib_flockfile_end (fp);
  return result;
}
