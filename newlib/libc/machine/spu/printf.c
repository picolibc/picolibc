#include <_ansi.h>
#include <stdio.h>

#include "c99ppe.h"

#ifdef _HAVE_STDC
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#ifdef INTEGER_ONLY
#  define printf iprintf
#endif

typedef struct
{
  _CONST char* fmt;
  unsigned int pad0[ 3 ];
  va_list ap;
} c99_printf_t;

#ifndef _REENT_ONLY

#ifdef _HAVE_STDC
int
_DEFUN (printf, (fmt,ap),
	_CONST char *fmt _AND
	...)
#else
int
#error
printf (fmt, va_alist)
     _CONST char *fmt;
     va_dcl
#endif
{
  int* ret;
  c99_printf_t args;

  CHECK_STD_INIT(_REENT);

  ret = (int*) &args;

  args.fmt = fmt;
#ifdef _HAVE_STDC
  va_start (args.ap, fmt);
#else
  va_start (args.ap);
#endif

  send_to_ppe(SPE_C99_SIGNALCODE, SPE_C99_VPRINTF, &args);

  va_end (args.ap);
  return *ret;
}

#endif /* ! _REENT_ONLY */
