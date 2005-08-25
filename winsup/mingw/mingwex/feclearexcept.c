#include <fenv.h>

/* 7.6.2.1
   The feclearexcept function clears the supported exceptions
   represented by its argument.  */

int feclearexcept (int excepts)
{
  fenv_t _env;
  __asm__ volatile ("fnstenv %0;" : "=m" (_env)); /* get the env */
  _env.__status_word &= ~(excepts & FE_ALL_EXCEPT); /* clear the except */
  __asm__ volatile ("fldenv %0;" :: "m" (_env)); /*set the env */

  return 0;
}
