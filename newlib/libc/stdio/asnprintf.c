/* Copyright (C) 2007, 2008 Eric Blake
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */
/* This code was derived from asprintf.c */
/* doc in sprintf.c */

#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>
#include "local.h"

char *
_asnprintf_r (struct _reent *__restrict ptr,
       char *buf,
       size_t *lenp,
       const char *__restrict fmt, ...)
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
      _REENT_ERRNO(ptr) = EOVERFLOW;
      return NULL;
    }
  f._bf._size = f._w = len;
  f._file = -1;  /* No file. */
  va_start (ap, fmt);
  ret = _svfprintf_r (ptr, &f, fmt, ap);
  va_end (ap);
  if (ret < 0)
    return NULL;
  *lenp = ret;
  *f._p = '\0';
  return (char *) f._bf._base;
}

#ifdef _NANO_FORMATTED_IO
char *
_asniprintf_r (struct _reent *, char *, size_t *, const char *, ...)
       _ATTRIBUTE ((__alias__("_asnprintf_r")));
#endif

#ifndef _REENT_ONLY

char *
asnprintf (char *__restrict buf,
       size_t *__restrict lenp,
       const char *__restrict fmt, ...)
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
      _REENT_ERRNO(ptr) = EOVERFLOW;
      return NULL;
    }
  f._bf._size = f._w = len;
  f._file = -1;  /* No file. */
  va_start (ap, fmt);
  ret = _svfprintf_r (ptr, &f, fmt, ap);
  va_end (ap);
  if (ret < 0)
    return NULL;
  *lenp = ret;
  *f._p = '\0';
  return (char *) f._bf._base;
}

#ifdef _NANO_FORMATTED_IO
char *
asniprintf (char *, size_t *, const char *, ...)
       _ATTRIBUTE ((__alias__("asnprintf")));
#endif
#endif /* ! _REENT_ONLY */
