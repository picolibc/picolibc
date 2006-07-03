#include <fenv.h>
#include "cpu_features.h"
/* 7.6.2.5 
   The fetestexcept function determines which of a specified subset of
   the exception flags are currently set. The excepts argument
   specifies the exception flags to be queried.
   The fetestexcept function returns the value of the bitwise OR of the
   exception macros corresponding to the currently set exceptions
   included in excepts. */

int fetestexcept (int excepts)
{
  
  unsigned int _res;
  __asm__ ("fnstsw %%ax" : "=a" (_res));


  /* If SSE supported, return the union of the FPU and SSE flags.  */
  if (__HAS_SSE)
    {     
        unsigned int _csr; 	
      __asm__ volatile("stmxcsr %0" : "=m" (_csr));
      _res |= _csr;    
    }

  return (_res & excepts & FE_ALL_EXCEPT);
}
