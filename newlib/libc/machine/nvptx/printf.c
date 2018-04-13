/*
 * Support file for nvptx in newlib.
 * Copyright (c) 2015-2018 Mentor Graphics.
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */

#include <stdarg.h>

extern int vprintf (const char *, va_list);

int
printf (const char *fmt, ...)
{
  va_list args;
  int res;

  va_start (args, fmt);
  res = vprintf (fmt, args);
  va_end (args);
  return res;
}
