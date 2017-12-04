/* Copyright (C) 2007, 2008 Eric Blake
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */
/* This code was derived from asprintf.c */
/* doc in viprintf.c */

#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>
#include "local.h"

char *
_vasniprintf_r (struct _reent *ptr,
       char *buf,
       size_t *lenp,
       const char *fmt,
       va_list ap)
{
  int ret;
  FILE f;
  size_t len = *lenp;

  if (buf && len)
    {
      /* mark an existing buffer, but allow allocation of larger string */
      f._flags = __SWR | __SSTR | __SOPT;
    }
  else
    {
      /* mark a zero-length reallocatable buffer */
      f._flags = __SWR | __SSTR | __SMBF;
      len = 0;
      buf = NULL;
    }
  f._bf._base = f._p = (unsigned char *) buf;
  /* For now, inherit the 32-bit signed limit of FILE._bf._size.
     FIXME - it would be nice to rewrite sys/reent.h to support size_t
     for _size.  */
  if (len > INT_MAX)
    {
      ptr->_errno = EOVERFLOW;
      return NULL;
    }
  f._bf._size = f._w = len;
  f._file = -1;  /* No file. */
  ret = _svfiprintf_r (ptr, &f, fmt, ap);
  if (ret < 0)
    return NULL;
  *lenp = ret;
  *f._p = '\0';
  return (char *) f._bf._base;
}

#ifndef _REENT_ONLY

char *
vasniprintf (char *buf,
       size_t *lenp,
       const char *fmt,
       va_list ap)
{
  return _vasniprintf_r (_REENT, buf, lenp, fmt, ap);
}

#endif /* ! _REENT_ONLY */
