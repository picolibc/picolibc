#include <fenv.h> 
/* 7.6.2.5 
   The fetestexcept function determines which of a specified subset of
   the exception flags are currently set. The excepts argument
   specifies the exception flags to be queried.
   The fetestexcept function returns the value of the bitwise OR of the
   exception macros corresponding to the currently set exceptions
   included in excepts. */

int fetestexcept (int excepts)
{
  unsigned short _sw;
  __asm__ ("fnstsw %%ax" : "=a" (_sw));
  return _sw & excepts & FE_ALL_EXCEPT;
}
