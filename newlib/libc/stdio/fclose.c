/*
FUNCTION
<<fclose>>---close a file

INDEX
	fclose

ANSI_SYNOPSIS
	#include <stdio.h>
	int fclose(FILE *<[fp]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	int fclose(<[fp]>)
	FILE *<[fp]>;

DESCRIPTION
If the file or stream identified by <[fp]> is open, <<fclose>> closes
it, after first ensuring that any pending data is written (by calling
<<fflush(<[fp]>)>>).

RETURNS
<<fclose>> returns <<0>> if successful (including when <[fp]> is
<<NULL>> or not an open file); otherwise, it returns <<EOF>>.

PORTABILITY
<<fclose>> is required by ANSI C.

Required OS subroutines: <<close>>, <<fstat>>, <<isatty>>, <<lseek>>,
<<read>>, <<sbrk>>, <<write>>.
*/

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

#include <stdio.h>
#include <stdlib.h>
#include "local.h"

/*
 * Close a file.
 */

int
_DEFUN (fclose, (fp),
	register FILE * fp)
{
  int r;

  if (fp == NULL)
    return (0);			/* on NULL */

  CHECK_INIT (fp);

  if (fp->_flags == 0)		/* not open! */
    return (0);
  r = fp->_flags & __SWR ? fflush (fp) : 0;
  if (fp->_close != NULL && (*fp->_close) (fp->_cookie) < 0)
    r = EOF;
  if (fp->_flags & __SMBF)
    _free_r (fp->_data, (char *) fp->_bf._base);
  if (HASUB (fp))
    FREEUB (fp);
  if (HASLB (fp))
    FREELB (fp);
  fp->_flags = 0;		/* release this FILE for reuse */
  return (r);
}
