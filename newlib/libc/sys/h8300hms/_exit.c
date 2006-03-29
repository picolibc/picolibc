/* FIXME: which one? */

#include <_ansi.h>

/* `sleep' is passed an argument in r0 that indicates the reason
   the program is exiting.  The format of r0 is defined in devo/include/wait.h.
*/

void
_DEFUN (_exit,(rc),
     int rc)
{
  short rc2 = rc << 8;
  asm("mov.w %0,r0\n\tsleep" : : "r" (rc2) : "r0");
}

void
_DEFUN (__exit,(rc),
     int rc)
{
  short rc2 = rc << 8;
  asm("mov.w %0,r0\n\tsleep" : : "r" (rc2) : "r0");
}
