#include <fenv.h>
#include "cpu_features.h"

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

  if (__HAS_SSE)
    {
       unsigned int _csr;
       /* Save the SSE MXCSR register.  */
       __asm__ ("stmxcsr %0" :  "=m" (envp->__mxcsr));  
       /* Clear the exception flags. */
       _csr = envp->__mxcsr & ~FE_ALL_EXCEPT;
       /* Set exception mask to non-stop  */
       _csr |= (FE_ALL_EXCEPT << __MXCSR_EXCEPT_MASK_SHIFT) /*= 0x1f80 */;
       __asm__ volatile ("ldmxcsr %0" : : "m" (_csr));
    } 

  return 0;
}
