
#include <_ansi.h>
#include <stdio.h>

#include "local.h"

#ifdef _HAVE_STDC
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#ifdef _HAVE_STDC
int
_printf_r (struct _reent *ptr, const char *fmt, ...)
#else
int
_printf_r (ptr, fmt, va_alist)
     struct _reent *ptr;
     char *fmt;
     va_dcl
#endif
{
  int ret;
  va_list ap;

  _REENT_SMALL_CHECK_INIT(_stdout_r (ptr));
#ifdef _HAVE_STDC
  va_start (ap, fmt);
#else
  va_start (ap);
#endif
  ret = _vfprintf_r (ptr, _stdout_r (ptr), fmt, ap);
  va_end (ap);
  return ret;
}

#ifndef _REENT_ONLY

#ifdef _HAVE_STDC
int
printf (const char *fmt, ...)
#else
int
printf (fmt, va_alist)
     char *fmt;
     va_dcl
#endif
{
  int ret;
  va_list ap;

  _REENT_SMALL_CHECK_INIT(_stdout_r (_REENT));
#ifdef _HAVE_STDC
  va_start (ap, fmt);
#else
  va_start (ap);
#endif
  ret = vfprintf (_stdout_r (_REENT), fmt, ap);
  va_end (ap);
  return ret;
}

#endif /* ! _REENT_ONLY */
