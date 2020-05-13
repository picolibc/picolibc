/*
Copyright (c) 2014-2017 Mentor Graphics.

The authors hereby grant permission to use, copy, modify, distribute,
and license this software and its documentation for any purpose, provided
that existing copyright notices are retained in all copies and that this
notice is included verbatim in any distributions. No written agreement,
license, or royalty fee is required for any of the authorized uses.
Modifications to this software may be copyrighted by their authors
and need not follow the licensing terms described here, provided that
the new terms are clearly indicated on the first page of each file where
they apply.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <newlib.h>

int write (int fd, const char *buf, int len);

int
__wrap_puts (const char * s)
{
  int res = write (1, s, strlen (s));
  if (res == EOF)
  {
    write (1, "\n", 1);
    return EOF;
  }
  return write (1, "\n", 1);
}

int puts (const char * s) __attribute__ ((weak, alias ("__wrap_puts")));
