#include <sys/dos.h>

int
intdos(union REGS *in, union REGS *out)
{
  return int86(0x21, in, out);
}
