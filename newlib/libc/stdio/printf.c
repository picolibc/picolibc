
#include <_ansi.h>
#include <stdio.h>

#ifdef _HAVE_STDC

#include <stdarg.h>

int
_printf_r (struct _reent *ptr, const char *fmt, ...)
{
  int ret;
  va_list ap;

  va_start (ap, fmt);
  ret = _vfprintf_r (ptr, _stdout_r (ptr), fmt, ap);
  va_end (ap);
  return ret;
}

#else

#include <varargs.h>

int
_printf_r (ptr, fmt, va_alist)
     struct _reent *ptr;
     char *fmt;
     va_dcl
{
  int ret;
  va_list ap;

  va_start (ap);
  ret = _vfprintf_r (ptr, _stdout_r (ptr), fmt, ap);
  va_end (ap);
  return ret;
}

#endif


#ifndef _REENT_ONLY

#ifdef _HAVE_STDC

#include <stdarg.h>

int
printf (const char *fmt, ...)
{
  int ret;
  va_list ap;

  va_start (ap, fmt);
  _stdout_r (_REENT)->_data = _REENT;
  ret = vfprintf (_stdout_r (_REENT), fmt, ap);
  va_end (ap);
  return ret;
}

#else

#include <varargs.h>

int
printf (fmt, va_alist)
     char *fmt;
     va_dcl
{
  int ret;
  va_list ap;

  va_start (ap);
  _stdout_r (_REENT)->_data = _REENT;
  ret = vfprintf (_stdout_r (_REENT), fmt, ap);
  va_end (ap);
  return ret;
}

#endif /* ! _HAVE_STDC */

#endif /* ! _REENT_ONLY */
