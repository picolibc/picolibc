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
<<fread>>---read array elements from a file

INDEX
	fread

ANSI_SYNOPSIS
	#include <stdio.h>
	size_t fread(void *<[buf]>, size_t <[size]>, size_t <[count]>,
		     FILE *<[fp]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	size_t fread(<[buf]>, <[size]>, <[count]>, <[fp]>)
	char *<[buf]>;
	size_t <[size]>;
	size_t <[count]>;
	FILE *<[fp]>;

DESCRIPTION
<<fread>> attempts to copy, from the file or stream identified by
<[fp]>, <[count]> elements (each of size <[size]>) into memory,
starting at <[buf]>.   <<fread>> may copy fewer elements than
<[count]> if an error, or end of file, intervenes.

<<fread>> also advances the file position indicator (if any) for
<[fp]> by the number of @emph{characters} actually read.

RETURNS
The result of <<fread>> is the number of elements it succeeded in
reading.

PORTABILITY
ANSI C requires <<fread>>.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#include <stdio.h>
#include <string.h>
#include "local.h"

size_t
_DEFUN (fread, (buf, size, count, fp),
	_PTR buf _AND
	size_t size _AND
	size_t count _AND
	FILE * fp)
{
  register size_t resid;
  register char *p;
  register int r;
  size_t total;

  if ((resid = count * size) == 0)
    return 0;
  if (fp->_r < 0)
    fp->_r = 0;
  total = resid;
  p = buf;
  while (resid > (r = fp->_r))
    {
      (void) memcpy ((void *) p, (void *) fp->_p, (size_t) r);
      fp->_p += r;
      /* fp->_r = 0 ... done in __srefill */
      p += r;
      resid -= r;
      if (__srefill (fp))
	{
	  /* no more input: return partial result */
	  return (total - resid) / size;
	}
    }
  (void) memcpy ((void *) p, (void *) fp->_p, resid);
  fp->_r -= resid;
  fp->_p += resid;
  return count;
}
