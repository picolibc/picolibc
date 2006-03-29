#include <errno.h>
#include "dos.h"

bdos(int func, unsigned dx, unsigned al)
{
  union REGS r;
  r.x.dx = dx;
  r.h.ah = func;
  r.h.al = al;
  int86(0x21, &r, &r);
  return r.x.ax;
}
