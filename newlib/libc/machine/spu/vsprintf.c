
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
  char *str;
  unsigned int pad0[ 3 ];
  char *fmt;
  unsigned int pad1[ 3 ];
  va_list ap;
} c99_vsprintf_t;

int
_DEFUN (vsprintf, (str, fmt, ap),
     char *str _AND
     _CONST char *fmt _AND
     va_list ap)
{
  int* ret;
  c99_vsprintf_t args;
  ret = (int*) &args;

  args.str = str;
  args.fmt = (char*) fmt;
  va_copy(args.ap,ap);

  send_to_ppe(SPE_C99_SIGNALCODE, SPE_C99_VSPRINTF, &args);

  return *ret;
}
