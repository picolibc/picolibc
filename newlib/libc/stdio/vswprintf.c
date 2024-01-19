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
/* doc in vfwprintf.c */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "%W% (Berkeley) %G%";
#endif /* LIBC_SCCS and not lint */

#define _DEFAULT_SOURCE
#include <_ansi.h>
#include <stdio.h>
#include <wchar.h>
#include <limits.h>
#include <stdarg.h>
#include <errno.h>

#include "local.h"

int
vswprintf (
       wchar_t *str,
       size_t size,
       const wchar_t *fmt,
       va_list ap)
{
  int ret;
  FILE f;

  if (size > INT_MAX / sizeof (wchar_t))
    {
      _REENT_ERRNO(ptr) = EOVERFLOW;	/* POSIX extension */
      return EOF;
    }
  f._flags = __SWR | __SSTR;
  f._flags2 = 0;
  f._bf._base = f._p = (unsigned char *) str;
  f._bf._size = f._w = (size > 0 ? (size - 1) * sizeof (wchar_t) : 0);
  f._file = -1;  /* No file. */
  ret = svfwprintf ( &f, fmt, ap);
  /* svfwprintf( so add one if
   * appropriate, which is whenever size is > 0.  _svfwprintf_r() stops
   * after n-1, so always just put at the end.  */
  if (size > 0)  {
    *(wchar_t *)f._p = L'\0';	/* terminate the string */
  }
  if(ret >= 0 && (size_t) ret >= size)  {
    /* _svfwprintf_r() returns how many wide characters it would have printed
     * if there were enough space.  Return an error if too big to fit in str,
     * unlike snprintf, which returns the size needed.  */
    _REENT_ERRNO(ptr) = EOVERFLOW;	/* POSIX extension */
    ret = -1;
  }
  return ret;
}
