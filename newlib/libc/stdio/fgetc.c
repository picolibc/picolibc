/*
FUNCTION
<<fgetc>>---get a character from a file or stream

INDEX
	fgetc

ANSI_SYNOPSIS
	#include <stdio.h>
	int fgetc(FILE *<[fp]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	int fgetc(<[fp]>)
	FILE *<[fp]>;

DESCRIPTION
Use <<fgetc>> to get the next single character from the file or stream
identified by <[fp]>.  As a side effect, <<fgetc>> advances the file's
current position indicator.

For a macro version of this function, see <<getc>>.

RETURNS
The next character (read as an <<unsigned char>>, and cast to
<<int>>), unless there is no more data, or the host system reports a
read error; in either of these situations, <<fgetc>> returns <<EOF>>.

You can distinguish the two situations that cause an <<EOF>> result by
using the <<ferror>> and <<feof>> functions.

PORTABILITY
ANSI C requires <<fgetc>>.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#include <stdio.h>

int
_DEFUN (fgetc, (fp),
	FILE * fp)
{
  return __sgetc (fp);
}
