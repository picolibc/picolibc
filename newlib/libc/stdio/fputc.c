/*
FUNCTION
<<fputc>>---write a character on a stream or file

INDEX
	fputc

ANSI_SYNOPSIS
	#include <stdio.h>
	int fputc(int <[ch]>, FILE *<[fp]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	int fputc(<[ch]>, <[fp]>)
	int <[ch]>;
	FILE *<[fp]>;

DESCRIPTION
<<fputc>> converts the argument <[ch]> from an <<int>> to an
<<unsigned char>>, then writes it to the file or stream identified by
<[fp]>.

If the file was opened with append mode (or if the stream cannot
support positioning), then the new character goes at the end of the
file or stream.  Otherwise, the new character is written at the
current value of the position indicator, and the position indicator
oadvances by one.

For a macro version of this function, see <<putc>>.

RETURNS
If successful, <<fputc>> returns its argument <[ch]>.  If an error
intervenes, the result is <<EOF>>.  You can use `<<ferror(<[fp]>)>>' to
query for errors.

PORTABILITY
<<fputc>> is required by ANSI C.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#include <stdio.h>

int
_DEFUN (fputc, (ch, file),
	int ch _AND
	FILE * file)
{
  return putc (ch, file);
}
