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
/* This code was copied from sprintf.c */

#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#ifdef _HAVE_STDC
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include <limits.h>
#include "local.h"

int
#ifdef _HAVE_STDC
_DEFUN(_asprintf_r, (ptr, strp, fmt),
       struct _reent *ptr _AND
       char **strp        _AND
       _CONST char *fmt _DOTS)
#else
_asprintf_r(ptr, strp, fmt, va_alist)
           struct _reent *ptr;
           char **strp;
           _CONST char *fmt;
           va_dcl
#endif
{
  int ret;
  va_list ap;
  FILE f;

  /* mark a zero-length reallocatable buffer */
  f._flags = __SWR | __SSTR | __SMBF;
  f._bf._base = f._p = NULL;
  f._bf._size = f._w = 0;
  f._file = -1;  /* No file. */
#ifdef _HAVE_STDC
  va_start (ap, fmt);
#else
  va_start (ap);
#endif
  ret = vfprintf (&f, fmt, ap);
  va_end (ap);
  *f._p = 0;
  *strp = f._bf._base;
  return (ret);
}

#ifndef _REENT_ONLY

int
#ifdef _HAVE_STDC
_DEFUN(asprintf, (strp, fmt),
       char **strp _AND
       _CONST char *fmt _DOTS)
#else
asprintf(strp, fmt, va_alist)
        char **strp;
        _CONST char *fmt;
        va_dcl
#endif
{
  int ret;
  va_list ap;
  FILE f;
  
  /* mark a zero-length reallocatable buffer */
  f._flags = __SWR | __SSTR | __SMBF;
  f._bf._base = f._p = NULL;
  f._bf._size = f._w = 0;
  f._file = -1;  /* No file. */
#ifdef _HAVE_STDC
  va_start (ap, fmt);
#else
  va_start (ap);
#endif
  ret = vfprintf (&f, fmt, ap);
  va_end (ap);
  *f._p = 0;
  *strp = f._bf._base;
  return (ret);
}

#endif
