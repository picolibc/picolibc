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
#include <stdio.h>

#ifdef _HAVE_STDC

#include <stdarg.h>

extern int __svfscanf ();

int
fscanf (FILE * fp, const char *fmt, ...)
{
  int ret;
  va_list ap;

  va_start (ap, fmt);
  ret = __svfscanf (fp, fmt, ap);
  va_end (ap);
  return ret;
}

#else

#include <varargs.h>

extern int __svfscanf ();

int
fscanf (fp, fmt, va_alist)
     FILE *fp;
     char *fmt;
     va_dcl
{
  int ret;
  va_list ap;

  va_start (ap);
  ret = __svfscanf (fp, fmt, ap);
  va_end (ap);
  return ret;
}

#endif
