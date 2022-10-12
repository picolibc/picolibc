/* Copyright 2005, 2007 Shaun Jackman
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */
/* doc in dprintf.c */

#define _DEFAULT_SOURCE
#include <_ansi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include "local.h"

int
vdprintf (
       int fd,
       const char *__restrict format,
       va_list ap)
{
  char *p;
  char buf[512];
  size_t n = sizeof buf;

  _REENT_SMALL_CHECK_INIT (ptr);
  p = vasnprintf ( buf, &n, format, ap);
  if (!p)
    return -1;
  n = write (fd, p, n);
  if (p != buf)
    free (p);
  return n;
}

#ifdef _NANO_FORMATTED_IO
int
vdiprintf (int, const char *, __VALIST)
       _ATTRIBUTE ((__alias__("vdprintf")));
#endif
