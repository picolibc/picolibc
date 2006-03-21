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

#include <_ansi.h>
#include <stdio.h>

int
_DEFUN(fputc, (ch, file),
       int ch _AND
       FILE * file)
{
  int result;
   _flockfile (file);
  result = putc (ch, file);
  _funlockfile (file);
  return result;
}
