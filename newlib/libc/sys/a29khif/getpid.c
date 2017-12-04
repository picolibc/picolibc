/* Stub for getpid.  */

#include <_ansi.h>

/* This should really return pid_t, but that doesn't seem to be in
   <sys/types.h>.  */

int
_getpid (void)
{
  return 1;
}
