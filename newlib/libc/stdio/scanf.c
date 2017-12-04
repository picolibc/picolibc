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

#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include <stdarg.h>
#include "local.h"

#ifndef _REENT_ONLY

int
scanf(const char *__restrict fmt, ...)
{
  int ret;
  va_list ap;
  struct _reent *reent = _REENT;

  _REENT_SMALL_CHECK_INIT (reent);
  va_start (ap, fmt);
  ret = _vfscanf_r (reent, _stdin_r (reent), fmt, ap);
  va_end (ap);
  return ret;
}

#ifdef _NANO_FORMATTED_IO
int
iscanf (const char *, ...)
       _ATTRIBUTE ((__alias__("scanf")));
#endif

#endif /* !_REENT_ONLY */

int
_scanf_r(struct _reent *ptr, const char *__restrict fmt, ...)
{
  int ret;
  va_list ap;

  _REENT_SMALL_CHECK_INIT (ptr);
  va_start (ap, fmt);
  ret = _vfscanf_r (ptr, _stdin_r (ptr), fmt, ap);
  va_end (ap);
  return (ret);
}

#ifdef _NANO_FORMATTED_IO
int
_iscanf_r (struct _reent *, const char *, ...)
       _ATTRIBUTE ((__alias__("_scanf_r")));
#endif
