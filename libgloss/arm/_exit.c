#include <_ansi.h>
#include "swi.h"

int _kill_shared (int, int, int) __attribute__((__noreturn__));
void _exit (int);

void
_exit (int status)
{
  /* The same SWI is used for both _exit and _kill.
     For _exit, call the SWI with "reason" set to ADP_Stopped_ApplicationExit
     to mark a standard exit.
     Note: The RDI implementation of _kill_shared throws away all its
     arguments and all implementations ignore the first argument.  */
  _kill_shared (-1, status, ADP_Stopped_ApplicationExit);
}
