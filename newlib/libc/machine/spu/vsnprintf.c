
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
  char* str;
  unsigned int pad0[ 3 ];
  size_t size;
  unsigned int pad1[ 3 ];
  char* fmt;
  unsigned int pad2[ 3 ];
  va_list ap;
} c99_vsnprintf_t;

#ifndef _REENT_ONLY

int
_DEFUN (vsnprintf, (str, size, fmt, ap),
     char *str _AND
     size_t size _AND
     _CONST char *fmt _AND
     va_list ap)
{
  int* ret;
  c99_vsnprintf_t args;
  ret = (int*) &args;

  args.str = str;
  args.size = size;
  args.fmt = fmt;
  va_copy(args.ap,ap);

  send_to_ppe(SPE_C99_SIGNALCODE, SPE_C99_VSNPRINTF, &args);

  return *ret;
}

#endif /* ! _REENT_ONLY */
