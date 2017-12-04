#include <_ansi.h>

char *
_DEFUN(_user_strerror, (errnum, internal, errptr),
       int errnum,
       int internal,
       int *errptr)
{
  /* prevent warning about unused parameters */
  _CAST_VOID errnum;
  _CAST_VOID internal;
  _CAST_VOID errptr;

  return 0;
}
