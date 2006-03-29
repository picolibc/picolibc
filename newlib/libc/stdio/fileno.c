/*
FUNCTION
<<fileno>>---return file descriptor associated with stream

INDEX
	fileno

ANSI_SYNOPSIS
	#include <stdio.h>
	int fileno(FILE *<[fp]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	int fileno(<[fp]>)
	FILE *<[fp]>;

DESCRIPTION
You can use <<fileno>> to return the file descriptor identified by <[fp]>.

RETURNS
<<fileno>> returns a non-negative integer when successful.
If <[fp]> is not an open stream, <<fileno>> returns -1.

PORTABILITY
<<fileno>> is not part of ANSI C.
POSIX requires <<fileno>>.

Supporting OS subroutines required: none.
*/

#include <stdio.h>
#include "local.h"

int
_DEFUN (fileno, (f),
	FILE * f)
{
  CHECK_INIT (f);
  return __sfileno (f);
}
