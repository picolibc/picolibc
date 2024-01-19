/*
 * Copyright (c) 1990, 2007 The Regents of the University of California.
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
/* This code was based on vsprintf.c */
/* doc in vfprintf.c */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "%W% (Berkeley) %G%";
#endif /* LIBC_SCCS and not lint */

#define _DEFAULT_SOURCE
#include <_ansi.h>
#include <stdio.h>
#include <limits.h>
#include <stdarg.h>

#include "local.h"

int
vasprintf (
       char **strp,
       const char *fmt,
       va_list ap)
{
  int ret;
  FILE f;

  f._flags = __SWR | __SSTR | __SMBF ;
  f._flags2 = 0;
  f._bf._base = f._p = NULL;
  f._bf._size = f._w = 0;
  f._file = -1;  /* No file. */
  ret = svfprintf ( &f, fmt, ap);
  if (ret >= 0)
    {
      *f._p = 0;
      *strp = (char *) f._bf._base;
    }
  return ret;
}

#ifdef _NANO_FORMATTED_IO
int
vasiprintf (char **, const char *, __VALIST)
       _ATTRIBUTE ((__alias__("vasprintf")));
#endif
