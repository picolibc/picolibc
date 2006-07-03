#include <fenv.h>
#include "cpu_features.h"

/* 7.6.2.1
   The feclearexcept function clears the supported exceptions
   represented by its argument.  */

int feclearexcept (int excepts)
{
  fenv_t _env;
  excepts &= FE_ALL_EXCEPT;
  __asm__ volatile ("fnstenv %0;" : "=m" (_env)); /* get the env */
  _env.__status_word &= ~excepts; /* clear the except */
  __asm__ volatile ("fldenv %0;" :: "m" (_env)); /*set the env */

  if (__HAS_SSE)
    {
       unsigned _csr;
       __asm__ volatile("stmxcsr %0" : "=m" (_csr)); /* get the register */
       _csr &= ~excepts; /* clear the except */
       __asm__ volatile("ldmxcsr %0" : : "m" (_csr)); /* set the register */
    }
  return 0;
}
