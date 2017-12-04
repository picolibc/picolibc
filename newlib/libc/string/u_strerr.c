#include <_ansi.h>

char *
_user_strerror (int errnum,
       int internal,
       int *errptr)
{
  /* prevent warning about unused parameters */
  (void) errnum;
  (void) internal;
  (void) errptr;

  return 0;
}
