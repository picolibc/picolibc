/* Copyright (C) 2007, 2008 Eric Blake
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */
/* This code was derived from asprintf.c */
/* doc in siprintf.c */

#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>
#include "local.h"

char *
_asniprintf_r (struct _reent *ptr,
       char *buf,
       size_t *lenp,
       const char *fmt, ...)
{
  int ret;
  va_list ap;
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
  va_start (ap, fmt);
  ret = _svfiprintf_r (ptr, &f, fmt, ap);
  va_end (ap);
  if (ret < 0)
    return NULL;
  *lenp = ret;
  *f._p = '\0';
  return (char *) f._bf._base;
}

#ifndef _REENT_ONLY

char *
asniprintf (char *buf,
       size_t *lenp,
       const char *fmt, ...)
{
  int ret;
  va_list ap;
  FILE f;
  size_t len = *lenp;
  struct _reent *ptr = _REENT;

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
  va_start (ap, fmt);
  ret = _svfiprintf_r (ptr, &f, fmt, ap);
  va_end (ap);
  if (ret < 0)
    return NULL;
  *lenp = ret;
  *f._p = '\0';
  return (char *) f._bf._base;
}

#endif /* ! _REENT_ONLY */
