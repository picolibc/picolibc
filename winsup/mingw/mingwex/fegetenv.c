#include <fenv.h>

/* 7.6.4.1
   The fegetenv function stores the current floating-point environment
   in the object pointed to by envp. */

int fegetenv (fenv_t * envp)
{
  __asm__ ("fnstenv %0;": "=m" (*envp));
  return 0;
}

