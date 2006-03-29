#include <sys/dos.h>

intdosx(union REGS *in, union REGS *out, struct SREGS *seg)
{
  return int86x(0x21, in, out, seg);
}
