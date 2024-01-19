/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * and/or other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/* No user fns here.  Pesch 15apr92. */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "%W% (Berkeley) %G%";
#endif /* LIBC_SCCS and not lint */

#include <_ansi.h>
#include <stdio.h>
#include <errno.h>
#include "local.h"
#include "fvwrite.h"

/*
 * Note that this is the same function as __swbuf, just to be called
 * from wide-char functions!
 *
 * The only difference is that we set and test the orientation differently.
 */

int
__swbufw (
       register int c,
       register FILE *fp)
{
  register int n;

  fp->_w = fp->_lbfsize;
  if (cantwrite (ptr, fp))
    return EOF;
  c = (unsigned char) c;

  if (ORIENT (fp, 1) != 1)
    return EOF;

  n = fp->_p - fp->_bf._base;
  if (n >= fp->_bf._size)
    {
      if (fflush (fp))
	return EOF;
      n = 0;
    }
  fp->_w--;
  *fp->_p++ = c;
  if (++n == fp->_bf._size || (fp->_flags & __SLBF && c == '\n'))
    if (fflush (fp))
      return EOF;
  return c;
}
