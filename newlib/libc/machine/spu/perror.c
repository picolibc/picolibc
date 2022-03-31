#include <stdio.h>
#include <errno.h>

#include "c99ppe.h"

#ifndef _REENT_ONLY

typedef struct
{
  const char* str;
  unsigned int pad0[ 3 ];
  int arg_errno;
  unsigned int pad1[ 3 ];
} c99_perror_t;

void
perror (const char *s)

{
  c99_perror_t arg;

  arg.str = s;
  arg.arg_errno = errno;
  __send_to_ppe(SPE_C99_SIGNALCODE, SPE_C99_PERROR, &arg);

  return;
}
#endif /* ! _REENT_ONLY */
