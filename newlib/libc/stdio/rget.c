/* No user fns here. Pesch 15apr92. */

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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "%W% (Berkeley) %G%";
#endif /* LIBC_SCCS and not lint */

#include <stdio.h>
#include "local.h"

/*
 * Handle getc() when the buffer ran out:
 * Refill, then return the first character
 * in the newly-filled buffer.
 */

int
__srget (fp)
     register FILE *fp;
{
  if (__srefill (fp) == 0)
    {
      fp->_r--;
      return *fp->_p++;
    }
  return EOF;
}
