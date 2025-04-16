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
/* doc in vfprintf.c */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "%W% (Berkeley) %G%";
#endif /* LIBC_SCCS and not lint */

#define _DEFAULT_SOURCE
#include <stdio.h>
#include <limits.h>
#include <stdarg.h>
#include <errno.h>

#include "local.h"

int
vsnprintf (
       char *__restrict str,
       size_t size,
       const char *__restrict fmt,
       va_list ap)
{
  int ret;
  FILE f;

  if (size > INT_MAX)
    {
      errno = EOVERFLOW;
      return EOF;
    }
  f._flags = __SWR | __SSTR;
  f._flags2 = 0;
  f._bf._base = f._p = (unsigned char *) str;
  f._bf._size = f._w = (size > 0 ? size - 1 : 0);
  f._file = -1;  /* No file. */
  ret = svfprintf ( &f, fmt, ap);
  if (ret < EOF)
    errno = EOVERFLOW;
  if (size > 0)
    *f._p = 0;
  return ret;
}

__nano_reference(vsnprintf, vsniprintf);
__ieee128_reference(vsnprintf, __vsnprintfieee128);
