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
<<fseek>>, <<fseeko>>---set file position

INDEX
	fseek
INDEX
	fseeko
INDEX
	_fseek_r
INDEX
	_fseeko_r

SYNOPSIS
	#include <stdio.h>
	int fseek(FILE *<[fp]>, long <[offset]>, int <[whence]>);
	int fseeko(FILE *<[fp]>, off_t <[offset]>, int <[whence]>);
	int _fseek_r(struct _reent *<[ptr]>, FILE *<[fp]>,
	             long <[offset]>, int <[whence]>);
	int _fseeko_r(struct _reent *<[ptr]>, FILE *<[fp]>,
	             off_t <[offset]>, int <[whence]>);

DESCRIPTION
Objects of type <<FILE>> can have a ``position'' that records how much
of the file your program has already read.  Many of the <<stdio>> functions
depend on this position, and many change it as a side effect.

You can use <<fseek>>/<<fseeko>> to set the position for the file identified by
<[fp]>.  The value of <[offset]> determines the new position, in one
of three ways selected by the value of <[whence]> (defined as macros
in `<<stdio.h>>'):

<<SEEK_SET>>---<[offset]> is the absolute file position (an offset
from the beginning of the file) desired.  <[offset]> must be positive.

<<SEEK_CUR>>---<[offset]> is relative to the current file position.
<[offset]> can meaningfully be either positive or negative.

<<SEEK_END>>---<[offset]> is relative to the current end of file.
<[offset]> can meaningfully be either positive (to increase the size
of the file) or negative.

See <<ftell>>/<<ftello>> to determine the current file position.

RETURNS
<<fseek>>/<<fseeko>> return <<0>> when successful.  On failure, the
result is <<EOF>>.  The reason for failure is indicated in <<errno>>:
either <<ESPIPE>> (the stream identified by <[fp]> doesn't support
repositioning) or <<EINVAL>> (invalid file position).

PORTABILITY
ANSI C requires <<fseek>>.

<<fseeko>> is defined by the Single Unix specification.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include <errno.h>
#include "local.h"

int
_fseek_r (struct _reent *ptr,
       register FILE *fp,
       long offset,
       int whence)
{
  return _fseeko_r (ptr, fp, offset, whence);
}

#ifndef _REENT_ONLY

int
fseek (register FILE *fp,
       long offset,
       int whence)
{
  return _fseek_r (_REENT, fp, offset, whence);
}

#endif /* !_REENT_ONLY */
