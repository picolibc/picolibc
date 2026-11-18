/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 Linumiz
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

/* FPSR fields: RM 19-18, XC 15-10, XE 9-5, XP 4-0 */
#define __FPSR_RM_SHIFT 18
#define __FPSR_RM_MASK  (3U << __FPSR_RM_SHIFT)
#define __FPSR_XC_SHIFT 10
#define __FPSR_XE_SHIFT 5
#define __FPSR_ENV_MASK (__FPSR_RM_MASK | (FE_ALL_EXCEPT << __FPSR_XE_SHIFT) | FE_ALL_EXCEPT)

#define __v850_get_fpsr()                                 \
    ({                                                    \
        unsigned int __fpsr;                              \
        __asm__ volatile("stsr fpsr, %0" : "=r"(__fpsr)); \
        __fpsr;                                           \
    })

#define __v850_set_fpsr(__fpsr) __asm__ volatile("ldsr %0, fpsr" : : "r"(__fpsr))

#ifdef __V850_FXU__

/* FXSR has the FPSR layout for these fields; flags are raised in FPSR */
#define __v850_get_fxsr()                                 \
    ({                                                    \
        unsigned int __fxsr;                              \
        __asm__ volatile("stsr fxsr, %0" : "=r"(__fxsr)); \
        __fxsr;                                           \
    })

#define __v850_set_fxsr(__fxsr) __asm__ volatile("ldsr %0, fxsr" : : "r"(__fxsr))

#define __v850_get_flags()      ((__v850_get_fpsr() | __v850_get_fxsr()) & FE_ALL_EXCEPT)

#define __v850_update(__clear, __set)                                                   \
    do {                                                                                \
        __v850_set_fpsr((__v850_get_fpsr() & ~(__clear)) | (__set));                    \
        __v850_set_fxsr((__v850_get_fxsr() & ~(__clear)) | ((__set) & ~FE_ALL_EXCEPT)); \
    } while (0)

#else

#define __v850_get_flags()            (__v850_get_fpsr() & FE_ALL_EXCEPT)

#define __v850_update(__clear, __set) __v850_set_fpsr((__v850_get_fpsr() & ~(__clear)) | (__set))

#endif

__declare_fenv_inline(int) feclearexcept(int excepts)
{
    unsigned int e = (unsigned int)excepts & FE_ALL_EXCEPT;
    __v850_update(e | (e << __FPSR_XC_SHIFT), 0);
    return 0;
}

__declare_fenv_inline(int) fegetexceptflag(fexcept_t *flagp, int excepts)
{
    *flagp = __v850_get_flags() & (unsigned int)excepts & FE_ALL_EXCEPT;
    return 0;
}

__declare_fenv_inline(int) fesetexceptflag(const fexcept_t *flagp, int excepts)
{
    unsigned int e = (unsigned int)excepts & FE_ALL_EXCEPT;
    __v850_update(e, *flagp & e);
    return 0;
}

__declare_fenv_inline(int) feraiseexcept(int excepts)
{
    __v850_update(0, (unsigned int)excepts & FE_ALL_EXCEPT);
    return 0;
}

__declare_fenv_inline(int) fesetexcept(int excepts)
{
    return feraiseexcept(excepts);
}

__declare_fenv_inline(int) fetestexcept(int excepts)
{
    return (int)(__v850_get_flags() & (unsigned int)excepts & FE_ALL_EXCEPT);
}

__declare_fenv_inline(int) fegetround(void)
{
    return (int)((__v850_get_fpsr() & __FPSR_RM_MASK) >> __FPSR_RM_SHIFT);
}

__declare_fenv_inline(int) fesetround(int rounding_mode)
{
    if ((unsigned int)rounding_mode > FE_DOWNWARD)
        return 1;
    __v850_update(__FPSR_RM_MASK, (unsigned int)rounding_mode << __FPSR_RM_SHIFT);
    return 0;
}

__declare_fenv_inline(int) fegetenv(fenv_t *envp)
{
    *envp = (__v850_get_fpsr() & __FPSR_ENV_MASK) | __v850_get_flags();
    return 0;
}

__declare_fenv_inline(int) fesetenv(const fenv_t *envp)
{
    __v850_update(__FPSR_ENV_MASK | (FE_ALL_EXCEPT << __FPSR_XC_SHIFT), *envp & __FPSR_ENV_MASK);
    return 0;
}

__declare_fenv_inline(int) feholdexcept(fenv_t *envp)
{
    fegetenv(envp);
    __v850_update(
        (FE_ALL_EXCEPT << __FPSR_XE_SHIFT) | (FE_ALL_EXCEPT << __FPSR_XC_SHIFT) | FE_ALL_EXCEPT, 0);
    return 0;
}

__declare_fenv_inline(int) feupdateenv(const fenv_t *envp)
{
    unsigned int raised = __v850_get_flags();
    fesetenv(envp);
    feraiseexcept((int)raised);
    return 0;
}

__declare_fenv_inline(int) feenableexcept(int excepts)
{
    return excepts == 0 ? 0 : -1;
}

__declare_fenv_inline(int) fedisableexcept(int excepts)
{
    return excepts == 0 ? 0 : -1;
}

__declare_fenv_inline(int) fegetexcept(void)
{
    return 0;
}
