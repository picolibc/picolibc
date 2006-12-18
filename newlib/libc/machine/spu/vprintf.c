
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
} c99_vprintf_t;

#ifndef _REENT_ONLY

int
_DEFUN (vprintf, (fmt, ap),
     _CONST char *fmt _AND
     va_list ap)
{
  int* ret;
  c99_vprintf_t args;
  ret = (int*) &args;

  args.fmt = (char*) fmt;
  va_copy(args.ap,ap);

  send_to_ppe(SPE_C99_SIGNALCODE, SPE_C99_VPRINTF, &args);

  return *ret;
}

#endif /* ! _REENT_ONLY */
