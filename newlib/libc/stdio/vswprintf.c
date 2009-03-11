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
/* doc in vfwprintf.c */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "%W% (Berkeley) %G%";
#endif /* LIBC_SCCS and not lint */

#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include <wchar.h>
#include <limits.h>
#include <stdarg.h>
#include <errno.h>
#include "local.h"

int
_DEFUN(_vswprintf_r, (ptr, str, size, fmt, ap),
       struct _reent *ptr _AND
       wchar_t *str          _AND
       size_t size        _AND
       const wchar_t *fmt   _AND
       va_list ap)
{
  int ret;
  FILE f;

  if (size > INT_MAX / sizeof (wchar_t))
    {
      ptr->_errno = EOVERFLOW;
      return EOF;
    }
  f._flags = __SWR | __SSTR;
  f._bf._base = f._p = (unsigned char *) str;
  f._bf._size = f._w = (size > 0 ? (size - 1) * sizeof (wchar_t) : 0);
  f._file = -1;  /* No file. */
  ret = _svfwprintf_r (ptr, &f, fmt, ap);
  if (ret < EOF)
    ptr->_errno = EOVERFLOW;
  if (size > 0)
    *f._p = 0;
  return ret;
}

#ifndef _REENT_ONLY

int
_DEFUN(vswprintf, (str, size, fmt, ap),
       wchar_t *str        _AND
       size_t size      _AND
       const wchar_t *fmt _AND
       va_list ap)
{
  return _vswprintf_r (_REENT, str, size, fmt, ap);
}

#endif /* !_REENT_ONLY */
