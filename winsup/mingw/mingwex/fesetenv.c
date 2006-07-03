#include <fenv.h>
#include <float.h>
#include "cpu_features.h"

/* 7.6.4.3
   The fesetenv function establishes the floating-point environment
   represented by the object pointed to by envp. The argument envp
   points to an object set by a call to fegetenv or feholdexcept, or
   equal the macro FE_DFL_ENV or an implementation-defined environment
   macro. Note that fesetenv merely installs the state of the exception
   flags represented through its argument, and does not raise these
   exceptions.
 */

extern void (*_imp___fpreset)( void ) ;

int fesetenv (const fenv_t * envp)
{
  /* Default mxcsr status is to mask all exceptions.  All other bits
     are zero.  */
     
  unsigned int _csr = FE_ALL_EXCEPT << __MXCSR_EXCEPT_MASK_SHIFT /*= 0x1f80 */; 
  
  if (envp == FE_PC64_ENV)
   /*
    *  fninit initializes the control register to 0x37f,
    *  the status register to zero and the tag word to 0FFFFh.
    *  The other registers are unaffected.
    */
    __asm__ ("fninit");

  else if (envp == FE_PC53_ENV)
   /*
    * MS _fpreset() does same *except* it sets control word
    * to 0x27f (53-bit precison).
    * We force calling _fpreset in msvcrt.dll
    */

   (*_imp___fpreset)();

  else if (envp == FE_DFL_ENV)
    /* Use the choice made at app startup */ 
    _fpreset();

  else
    {
      __asm__ ("fldenv %0;" : : "m" (*envp));
       /* Setting the reserved high order bits of MXCSR causes a segfault */
       _csr = envp ->__mxcsr & 0xffff;
    }

  /* Set MXCSR */   
   if (__HAS_SSE)
     __asm__ volatile ("ldmxcsr %0" : : "m" (_csr));
 
  return 0;
}
