#include <fenv.h>

/* 7.6.4.3
   The fesetenv function establishes the floating-point environment
   represented by the object pointed to by envp. The argument envp
   shall point to an object set by a call to fegetenv or feholdexcept,
   or equal the macro FE_DFL_ENV or an implementation-defined environment
   macro. Note that fesetenv merely installs the state of the exception
   flags represented through its argument, and does not raise these
   exceptions.  */

int fesetenv (const fenv_t * envp)
{
  if (envp == FE_DFL_ENV)
/* fninit initializes the control register to 0x37f,
   the status register to zero and the tag word to 0FFFFh.
   The other registers are unaffected */
    __asm__("fninit");	/* is   _fpreset () safer?  */
  else
    __asm__ ("fldenv %0;" : : "m" (*envp));
  return 0;
}
