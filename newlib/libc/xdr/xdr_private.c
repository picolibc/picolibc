/* xdr_private.c - utility functions for porting xdr
 *
 * Copyright (c) 2009 Charles S. Wilson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
#include <stdarg.h>
#include "xdr_private.h"

static xdr_vprintf_t xdr_vprintf = NULL;

xdr_vprintf_t
_DEFUN (xdr_set_vprintf, (fnptr),
        xdr_vprintf_t fnptr)
{
  xdr_vprintf_t tmp = xdr_vprintf;
  xdr_vprintf = fnptr;
  return tmp;
}

void
_DEFUN (xdr_vwarnx, (format, ap),
        _CONST char *format _AND
	va_list ap)
{
  if (xdr_vprintf)
    {
      (*xdr_vprintf)(format, ap);
      return;
    }
  /* otherwise, do nothing */
}

void
_DEFUN (xdr_warnx, (fmt),
        _CONST char *fmt _DOTS)
{
  va_list ap;
  va_start (ap, fmt);
  xdr_vwarnx (fmt, ap);
  va_end (ap);
}
