/* Copyright (c) 2011 Tensilica Inc.  ALL RIGHTS RESERVED.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
   FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
   TENSILICA INCORPORATED BE LIABLE FOR ANY DIRECT, INDIRECT,
   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
   STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
   OF THE POSSIBILITY OF SUCH DAMAGE.  */

__declare_fenv_inline(int) feclearexcept(int except)
{
  unsigned int fsr;

  except <<= _FE_EXCEPTION_FLAGS_OFFSET;
  __asm__ volatile ("rur.fsr %0" : "=a"(fsr));
  fsr = fsr & ~except;
  __asm__ volatile ("wur.fsr %0" : : "a"(fsr));
  return 0;
}

__declare_fenv_inline(int) fedisableexcept(int excepts)
{
  fexcept_t current;
  __asm__ volatile ("rur.fcr %0" : "=a"(current));
  current &= ~(excepts << _FE_EXCEPTION_ENABLE_OFFSET);
  __asm__ volatile ("wur.fcr %0" : "=a"(current));
  return 0;
}

__declare_fenv_inline(int) feenableexcept(int excepts)
{
  fexcept_t current;
  __asm__ volatile ("rur.fcr %0" : "=a"(current));
  current |= excepts << _FE_EXCEPTION_ENABLE_OFFSET;
  __asm__ volatile ("wur.fcr %0" : "=a"(current));
  return 0;
}

__declare_fenv_inline(int) fegetenv(fenv_t * env_ptr)
{
  unsigned int fsr;
  unsigned int fcr;
  __asm__ volatile ("rur.fsr %0" : "=a"(fsr));
  __asm__ volatile ("rur.fcr %0" : "=a"(fcr));
  *env_ptr = fsr | fcr;
  return 0;
}

__declare_fenv_inline(int) fegetexcept(void)
{
  fexcept_t current;
  __asm__ volatile ("rur.fsr %0" : "=a"(current));
  return (current >> _FE_EXCEPTION_ENABLE_OFFSET) & FE_ALL_EXCEPT;
}

__declare_fenv_inline(int) fegetexceptflag(fexcept_t *flagp, int excepts)
{
  unsigned int fsr;
  __asm__ volatile ("rur.fsr %0" : "=a"(fsr));
  fsr >>= _FE_EXCEPTION_FLAGS_OFFSET;
  excepts &= fsr;
  *flagp = excepts;

  return 0;
}

__declare_fenv_inline(int) fegetround(void)
{
  fexcept_t current;
  __asm__ volatile ("rur.fcr %0" : "=a"(current));
  return (current & _FE_ROUND_MODE_MASK) >> _FE_ROUND_MODE_OFFSET;
}

__declare_fenv_inline(int) feholdexcept(fenv_t * envp)
{
  fexcept_t fsr;
  fenv_t fcr;
  /* Get the environment.  */
  __asm__ volatile ("rur.fcr %0" : "=a"(fcr));
  __asm__ volatile ("rur.fsr %0" : "=a"(fsr));
  *envp = fsr | fcr;

  /* Clear the exception enable flags.  */
  fcr &= _FE_ROUND_MODE_MASK;
  __asm__ volatile ("wur.fcr %0" : :"a"(fcr));

  /* Clear the exception happened flags.  */
  fsr = 0;
  __asm__ volatile ("wur.fsr %0" : :"a"(fsr));

  return 0;
}

__declare_fenv_inline(int) feraiseexcept(int excepts)
{
  fexcept_t current;

  __asm__ volatile ("rur.fsr %0" : "=a"(current));
  current |= excepts << _FE_EXCEPTION_FLAGS_OFFSET;
  __asm__ volatile ("wur.fsr %0" : : "a"(current));
  return 0;
}

__declare_fenv_inline(int) fesetexcept(int excepts)
{
  return feraiseexcept(excepts);
}

__declare_fenv_inline(int) fesetenv(const fenv_t * env_ptr)
{
  __asm__ volatile ("wur.fsr %0" : : "a"(*env_ptr));
  __asm__ volatile ("wur.fcr %0" : : "a"(*env_ptr));
  return 0;
}

__declare_fenv_inline(int) fesetexceptflag(const fexcept_t *flagp, int excepts)
{
  unsigned int fsr;

  __asm__ volatile ("rur.fsr %0" : "=a"(fsr));
  fsr &= ~(excepts << _FE_EXCEPTION_FLAGS_OFFSET);
  fsr |= ((*flagp & excepts) << _FE_EXCEPTION_FLAGS_OFFSET);
  __asm__ volatile ("wur.fsr %0" : : "a"(fsr));
  return 0;
}

__declare_fenv_inline(int) fesetround(int round)
{
  __asm__ volatile ("wur.fcr %0" : : "a"(round));
  return 0;
}

__declare_fenv_inline(int) fetestexcept(int excepts)
{
  fexcept_t current;
  __asm__ volatile ("rur.fsr %0" : "=a"(current));
  return (current >> _FE_EXCEPTION_FLAGS_OFFSET) & excepts;
}

__declare_fenv_inline(int) feupdateenv(const fenv_t * envp)
{
  fenv_t current;
  fegetenv (&current);
  fesetenv (envp);
  return feraiseexcept (current);
}
