#include <_ansi.h>
#include <stdio.h>

#include "c99ppe.h"

#ifdef _HAVE_STDC
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#ifdef INTEGER_ONLY
#  define vsnprintf vsniprintf
#endif

typedef struct
{
  char* str;
  unsigned int pad0[ 3 ];
  size_t size;
  unsigned int pad1[ 3 ];
  const char* fmt;
  unsigned int pad2[ 3 ];
  va_list ap;
} c99_vsnprintf_t;

#ifndef _REENT_ONLY

int
_DEFUN (vsnprintf, (str, size, fmt, ap),
     char *__restrict str,
     size_t size,
     const char *__restrict fmt,
     va_list ap)
{
  c99_vsnprintf_t args;

  CHECK_STR_INIT(_REENT);

  args.str = str;
  args.size = size;
  args.fmt = fmt;
  va_copy(args.ap,ap);

  return __send_to_ppe(SPE_C99_SIGNALCODE, SPE_C99_VSNPRINTF, &args);
}

#endif /* ! _REENT_ONLY */
