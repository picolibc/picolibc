#include <fenv.h>

/* 7.6.4.2
   The feholdexcept function saves the current floating-point
   environment in the object pointed to by envp, clears the exception
   flags, and then installs a non-stop (continue on exceptions) mode,
   if available, for all exceptions.  */

int feholdexcept (fenv_t * envp)
{
  fenv_t tmp_env;
  __asm__ ("fnstenv %0;" : "=m" (* envp)); /* save current into envp */
  tmp_env = * envp;
  tmp_env.__status_word &= ~FE_ALL_EXCEPT; /* clear exception flags */
  tmp_env.__control_word |= FE_ALL_EXCEPT;  /* set cw to non-stop */
  __asm__ volatile ("fldenv %0;" : : "m" (tmp_env)); /* install the copy */
  return 0;
}
