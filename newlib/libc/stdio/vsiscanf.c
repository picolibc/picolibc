/*
 * Code created by modifying iscanf.c which has following copyright.
 *
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * and/or other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#define _DEFAULT_SOURCE
#include <_ansi.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "local.h"

/*
 * vsiscanf
 */

vsiscanf (
       const char *str,
       const char *fmt,
       va_list ap)
{
  FILE f;

  f._flags = __SRD | __SSTR;
  f._flags2 = 0;
  f._bf._base = f._p = (unsigned char *) str;
  f._bf._size = f._r = strlen (str);
  f._read = __seofread;
  f._ub._base = NULL;
  f._lb._base = NULL;
  f._file = -1;  /* No file. */
  return _ssvfiscanf ( &f, fmt, ap);
}
