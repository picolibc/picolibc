
#include <_ansi.h>
#include <stdio.h>

#include "c99ppe.h"

#ifdef _HAVE_STDC
#include <stdarg.h>
#else
#include <varargs.h>
#endif

typedef struct
{
  char* fmt;
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
     char *fmt;
     va_dcl
#endif
{
  int* ret;
  c99_printf_t args;
  ret = (int*) &args;

  args.fmt = fmt;
#ifdef _HAVE_STDC
  va_start (args.ap, args.fmt);
#else
  va_start (args.ap);
#endif


  /*  ret = vfprintf (_stdout_r (_REENT), fmt, ap);*/
  send_to_ppe(SPE_C99_SIGNALCODE, SPE_C99_VPRINTF, &args);

  va_end (args.ap);
  return *ret;
}

#endif /* ! _REENT_ONLY */
