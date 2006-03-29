#include "dos.h"

void gettime( struct time *tp)
{
  union REGS regs;
  regs.h.ah = 0x2c;
  intdos( &regs, &regs);
  tp->ti_hour = regs.h.ch;
  tp->ti_min = regs.h.cl;
  tp->ti_sec = regs.h.dh;
  tp->ti_hund = regs.h.dl;
}
