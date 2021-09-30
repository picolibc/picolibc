/* Copyright 2005, 2007 Shaun Jackman
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */
/* doc in diprintf.c */

#define _DEFAULT_SOURCE
#include <_ansi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include "local.h"

int
_vdiprintf_r (struct _reent *ptr,
       int fd,
       const char *format,
       va_list ap)
{
  char *p;
  char buf[512];
  size_t n = sizeof buf;

  _REENT_SMALL_CHECK_INIT (ptr);
  p = _vasniprintf_r (ptr, buf, &n, format, ap);
  if (!p)
    return -1;
  n = write (fd, p, n);
  if (p != buf)
    free (p);
  return n;
}

#ifndef _REENT_ONLY

int
vdiprintf (int fd,
       const char *format,
       va_list ap)
{
  return _vdiprintf_r (_REENT, fd, format, ap);
}

#endif /* ! _REENT_ONLY */
