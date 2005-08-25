#include <fenv.h>


/* 7.6.2.2  
   The fegetexceptflag function stores an implementation-defined
   representation of the exception flags indicated by the argument
   excepts in the object pointed to by the argument flagp.  */

int fegetexceptflag (fexcept_t * flagp, int excepts)
{
  unsigned short _sw;
  __asm__ ("fnstsw %%ax;": "=a" (_sw));
  *flagp = _sw  & excepts & FE_ALL_EXCEPT;
  return 0;
}
