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
fscanf(FILE *__restrict fp, const char *__restrict fmt, ...)
{
  int ret;
  va_list ap;

  va_start (ap, fmt);
  ret = _vfscanf_r (_REENT, fp, fmt, ap);
  va_end (ap);
  return ret;
}

#ifdef _NANO_FORMATTED_IO
int
fiscanf (FILE *, const char *, ...)
       _ATTRIBUTE ((__alias__("fscanf")));
#endif

#endif /* !_REENT_ONLY */

int
_fscanf_r(struct _reent *ptr, FILE *__restrict fp, const char *__restrict fmt, ...)
{
  int ret;
  va_list ap;

  va_start (ap, fmt);
  ret = _vfscanf_r (ptr, fp, fmt, ap);
  va_end (ap);
  return (ret);
}

#ifdef _NANO_FORMATTED_IO
int
_fiscanf_r (struct _reent *, FILE *, const char *, ...)
       _ATTRIBUTE ((__alias__("_fscanf_r")));
#endif
