#include <fenv.h>
#include "cpu_features.h"

/* 7.6.4.1
   The fegetenv function stores the current floating-point environment
   in the object pointed to by envp.  */

int fegetenv (fenv_t * envp)
{
  __asm__ ("fnstenv %0;": "=m" (*envp));
 /* fnstenv sets control word to non-stop for all exceptions, so we
    need to reload our env to restore the original mask.  */
  __asm__ ("fldenv %0" : : "m" (*envp));

  /* And the SSE environment.  */
  if (__HAS_SSE)
    __asm__ ("stmxcsr %0" : "=m" (envp->__mxcsr));

  return 0;
}
