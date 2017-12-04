#include <_ansi.h>

char *
_DEFUN(_user_strerror, (errnum, internal, errptr),
       int errnum,
       int internal,
       int *errptr)
{
  /* prevent warning about unused parameters */
  (void) errnum;
  (void) internal;
  (void) errptr;

  return 0;
}
