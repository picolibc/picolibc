/* Copyright (C) 2007, 2008 Eric Blake
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */
/* This code was derived from asprintf.c */
/* doc in siprintf.c */

#define _DEFAULT_SOURCE
#include <_ansi.h>
#include <stdio.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>
#include "local.h"

char *
asniprintf (char *buf,
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
  f._flags2 = 0;
  f._bf._base = f._p = (unsigned char *) buf;
  /* For now, inherit the 32-bit signed limit of FILE._bf._size.
     for _size.  */
  if (len > INT_MAX)
    {
      _REENT_ERRNO(ptr) = EOVERFLOW;
      return NULL;
    }
  f._bf._size = f._w = len;
  f._file = -1;  /* No file. */
  va_start (ap, fmt);
  ret = svfiprintf ( &f, fmt, ap);
  va_end (ap);
  if (ret < 0)
    return NULL;
  *lenp = ret;
  *f._p = '\0';
  return (char *) f._bf._base;
}
