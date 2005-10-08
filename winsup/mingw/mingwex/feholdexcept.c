#include <fenv.h>

/* 7.6.4.2
   The feholdexcept function saves the current floating-point
   environment in the object pointed to by envp, clears the exception
   flags, and then installs a non-stop (continue on exceptions) mode,
   if available, for all exceptions.  */

int feholdexcept (fenv_t * envp)
{
  __asm__ ("fnstenv %0;" : "=m" (* envp)); /* save current into envp */
 /* fnstenv sets control word to non-stop for all exceptions, so all we
    need to do is clear the exception flags.  */
  __asm__ ("fnclex");
  return 0;
}
