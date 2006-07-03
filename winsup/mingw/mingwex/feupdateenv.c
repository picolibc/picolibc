#include <fenv.h>
#include "cpu_features.h"

/* 7.6.4.4
   The feupdateenv function saves the currently raised exceptions in
   its automatic storage, installs the floating-point environment
   represented by the object pointed to by envp, and then raises the
   saved exceptions. The argument envp shall point to an object
   set by a call to feholdexcept or fegetenv, or equal the macro
   FE_DFL_ENV or an implementation-defined environment macro. */


int feupdateenv (const fenv_t * envp)
{
  unsigned int _fexcept;
  __asm__ ("fnstsw %%ax" : "=a" (_fexcept)); /*save excepts */
  if (__HAS_SSE)
    {
      unsigned int  _csr;
      __asm__ ("stmxcsr %0" : "=m" (_csr));
      _fexcept |= _csr;
    }
  fesetenv (envp); /* install the env  */
  feraiseexcept (_fexcept & FE_ALL_EXCEPT); /* raise the execeptions */
  return 0;
}
