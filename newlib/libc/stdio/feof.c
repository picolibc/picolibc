/*
FUNCTION
<<feof>>---test for end of file

INDEX
	feof

ANSI_SYNOPSIS
	#include <stdio.h>
	int feof(FILE *<[fp]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	int feof(<[fp]>)
	FILE *<[fp]>;

DESCRIPTION
<<feof>> tests whether or not the end of the file identified by <[fp]>
has been reached.

RETURNS
<<feof>> returns <<0>> if the end of file has not yet been reached; if
at end of file, the result is nonzero.

PORTABILITY
<<feof>> is required by ANSI C.

No supporting OS subroutines are required.
*/

#include <stdio.h>

#undef feof

int 
_DEFUN (feof, (fp),
	FILE * fp)
{
  return __sfeof (fp);
}
