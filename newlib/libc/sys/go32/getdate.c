#include "dos.h"

void getdate( struct date *dateblk)
{
  union REGS regs;
  regs.h.ah = 0x2a;
  intdos( &regs, &regs);
  dateblk-> da_year = regs.x.cx;
  dateblk-> da_mon = regs.h.dh;
  dateblk-> da_day = regs.h.dl;
}
