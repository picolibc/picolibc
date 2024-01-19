/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2023 Keith Packard
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define _M68K_EXCEPT_SHIFT      6

__declare_fenv_inline(int) feclearexcept(int excepts)
{
  /* Mask excepts to be sure only supported flag bits are set */

  excepts &= FE_ALL_EXCEPT;

  /* Clear the requested flags */

  fexcept_t fpsr;

  __asm__ volatile("fmove.l %%fpsr, %0" : "=d" (fpsr));
  fpsr &= ~excepts;
  __asm__ volatile("fmove.l %0, %%fpsr" : : "d" (fpsr));

  return 0;
}

__declare_fenv_inline(int) fegetenv(fenv_t *envp)
{

    /* Get the current fpcr and fpsr */

    fenv_t fpcr;
    fexcept_t fpsr;

    __asm__ volatile ("fmove.l %%fpcr, %0" : "=d"(fpcr));
    __asm__ volatile ("fmove.l %%fpsr, %0" : "=d"(fpsr));

    /* Mix the exceptions and rounding mode together */

    *envp = (fpcr & 0xff00) | ((fpcr >> 4) & 3) | (fpsr & 0xf8);

    return 0;
}

__declare_fenv_inline(int) fegetexceptflag(fexcept_t *flagp, int excepts)
{
    /* Mask excepts to be sure only supported flag bits are set */

    excepts &= FE_ALL_EXCEPT;

    /* Read the current fpsr */

    fexcept_t fpsr;

    __asm__ volatile("fmove.l %%fpsr, %0" : "=d"(fpsr));

    *flagp = (fpsr & excepts);

    return 0;
}

__declare_fenv_inline(int) fegetround(void)
{
    fenv_t fpcr;

    __asm__ volatile("fmove.l %%fpcr, %0" : "=d"(fpcr));

    return (fpcr >> 4) & 3;
}

__declare_fenv_inline(int) feholdexcept(fenv_t *envp)
{
    fenv_t fpcr;
    fexcept_t fpsr;

    __asm__ volatile("fmove.l %%fpcr, %0" : "=d"(fpcr));
    __asm__ volatile("fmove.l %%fpsr, %0" : "=d"(fpsr));

    int excepts = fpsr & FE_ALL_EXCEPT;

    *envp = (fpcr & 0xff00) | ((fpcr >> 4) & 3) | (fpsr & 0xf8);

    /* map except flags to except enables and clear them */

    fpcr &= ~(excepts << 6);

    fpsr &= ~excepts;

    /* Save to registers */

    __asm__ volatile("fmove.l %0, %%fpcr" : : "d"(fpcr));
    __asm__ volatile("fmove.l %0, %%fpsr" : : "d"(fpsr));

    return 0;
}

__declare_fenv_inline(int) feraiseexcept(int excepts)
{

    /* Mask excepts to be sure only supported flag bits are set */

    excepts &= FE_ALL_EXCEPT;

    /* Set the requested exception flags */

    fexcept_t fpsr;

    __asm__ volatile("fmove.l %%fpsr, %0" : "=d"(fpsr));

    fpsr |= excepts;

    __asm__ volatile("fmove.l %0, %%fpsr" : : "d"(fpsr));

    return 0;
}

__declare_fenv_inline(int) fesetexcept(int excepts)
{
    return feraiseexcept(excepts);
}

__declare_fenv_inline(int) fesetenv(const fenv_t *envp)
{
    fenv_t env = *envp;

    /* Get current fpcr and fpsr */

    fenv_t fpcr;
    fexcept_t fpsr;

    __asm__ volatile("fmove.l %%fpcr, %0" : "=d"(fpcr));
    __asm__ volatile("fmove.l %%fpsr, %0" : "=d"(fpsr));

    /* Set the rounding mode */

    fpcr = (fpcr & ~(0x3 << 4)) | (env & 3);

     /* Set the exception enables */

    fpcr = (fpcr & 0xff) | (env & 0xff00);

   /* Set the exceptions */

    fpsr = (fpsr & ~0xf8) | (env & 0xf8);

    /* Save to registers */

    __asm__ volatile("fmove.l %0, %%fpcr" : : "d"(fpcr));
    __asm__ volatile("fmove.l %0, %%fpsr" : : "d"(fpsr));

    return 0;

}

__declare_fenv_inline(int) fesetexceptflag(const fexcept_t *flagp, int excepts)
{
    excepts &= FE_ALL_EXCEPT;

    fexcept_t fpsr;

    __asm__ volatile("fmove.l %%fpsr, %0" : "=d"(fpsr));

    fpsr &= ~excepts;
    fpsr |= (*flagp & excepts);

    __asm__ volatile("fmove.l %0, %%fpsr" : : "d"(fpsr));

    return 0;
}

__declare_fenv_inline(int) fesetround(int rounding_mode)
{
    fenv_t fpcr;

    __asm__ volatile("fmove.l %%fpcr, %0" : "=d"(fpcr));

    fpcr = (fpcr & ~(3 << 4)) | ((rounding_mode & 3) << 4);

    __asm__ volatile("fmove.l %0, %%fpcr" : : "d"(fpcr));

    return 0;
}

__declare_fenv_inline(int) fetestexcept(int excepts)
{
  /* Mask excepts to be sure only supported flag bits are set */

  excepts &= FE_ALL_EXCEPT;

  /* Read the current fpsr */

  fexcept_t fpsr;

  __asm__ volatile ("fmove.l %%fpsr, %0" : "=d"(fpsr));

  return (fpsr & excepts);
}

__declare_fenv_inline(int) feupdateenv(const fenv_t *envp)
{
    fenv_t env = *envp;

    /* Get current fpcr and fpsr */

    fenv_t fpcr;
    fexcept_t fpsr;

    __asm__ volatile("fmove.l %%fpcr, %0" : "=d"(fpcr));
    __asm__ volatile("fmove.l %%fpsr, %0" : "=d"(fpsr));

    /* Set the rounding mode */

    fpcr = (fpcr & ~(0x3 << 4)) | (env & 3);

    /* Set the exception enables */

    fpcr = (fpcr & 0xff) | (env & 0xff00);

    /* Merge in exceptions */

    fpsr |= (env & 0xf8);

    /* Save to registers */

    __asm__ volatile("fmove.l %0, %%fpcr" : : "d"(fpcr));
    __asm__ volatile("fmove.l %0, %%fpsr" : : "d"(fpsr));

    return 0;

}

__declare_fenv_inline(int) feenableexcept(int excepts)
{
    fenv_t old_fpcr, new_fpcr;

    __asm__ volatile("fmove.l %%fpcr, %0" : "=d"(old_fpcr));

    /* Enable exceptions */

    new_fpcr = old_fpcr | ((excepts & FE_ALL_EXCEPT) << _M68K_EXCEPT_SHIFT);

    __asm__ volatile("fmove.l %0, %%fpcr" : : "d"(new_fpcr));
    return (old_fpcr >> _M68K_EXCEPT_SHIFT) & FE_ALL_EXCEPT;
}

__declare_fenv_inline(int) fedisableexcept(int excepts)
{
    fenv_t old_fpcr, new_fpcr;

    __asm__ volatile("fmove.l %%fpcr, %0" : "=d"(old_fpcr));

    /* Disable exceptions */

    new_fpcr = old_fpcr & ~((excepts & FE_ALL_EXCEPT) << _M68K_EXCEPT_SHIFT);

    __asm__ volatile("fmove.l %0, %%fpcr" : : "d"(new_fpcr));
    return (old_fpcr >> _M68K_EXCEPT_SHIFT) & FE_ALL_EXCEPT;
}

__declare_fenv_inline(int) fegetexcept(void)
{
    fenv_t fpcr;

    __asm__ volatile("fmove.l %%fpcr, %0" : "=d"(fpcr));

    return (fpcr >> _M68K_EXCEPT_SHIFT) & FE_ALL_EXCEPT;
}
