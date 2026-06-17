/*
Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
SPDX-License-Identifier: BSD-3-Clause-Clear

Redistribution and use in source and binary forms, with or without
modification, are permitted (subject to the limitations in the
disclaimer below) provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following
    disclaimer in the documentation and/or other materials provided
    with the distribution.

  * Neither the name of Qualcomm Technologies, Inc. nor the names of its
    contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* Hexagon hardware fenv inline implementations */

static __inline__ unsigned long
__hexagon_get_usr(void)
{
    unsigned long __r;
    __asm__ volatile("%0 = usr" : "=r"(__r));
    return __r;
}

static __inline__ void
__hexagon_set_usr(unsigned long __r)
{
    __asm__ volatile("usr = %0" ::"r"(__r));
}

__declare_fenv_inline(int) fegetenv(fenv_t *__envp)
{
    *__envp = (fenv_t)(__hexagon_get_usr() & _FE_FP_MASK);
    return 0;
}

__declare_fenv_inline(int) fesetenv(const fenv_t *__envp)
{
    unsigned long __usr = __hexagon_get_usr();
    __hexagon_set_usr((__usr & ~(unsigned long)_FE_FP_MASK)
                      | ((unsigned long)*__envp & _FE_FP_MASK));
    return 0;
}

__declare_fenv_inline(int) fegetround(void)
{
    return (int)((__hexagon_get_usr() >> _FE_RND_OFF) & _FE_RND_MASK);
}

__declare_fenv_inline(int) fesetround(int __round)
{
    unsigned long __usr;
    if ((unsigned int)__round > _FE_RND_MASK)
        return 1;
    __usr = __hexagon_get_usr();
    __hexagon_set_usr((__usr & ~((unsigned long)_FE_RND_MASK << _FE_RND_OFF))
                      | ((unsigned long)__round << _FE_RND_OFF));
    return 0;
}

__declare_fenv_inline(int) feclearexcept(int __excepts)
{
    __hexagon_set_usr(__hexagon_get_usr() & ~(unsigned long)(__excepts & FE_ALL_EXCEPT));
    return 0;
}

__declare_fenv_inline(int) feraiseexcept(int __excepts)
{
    __hexagon_set_usr(__hexagon_get_usr() | (unsigned long)(__excepts & FE_ALL_EXCEPT));
    return 0;
}

__declare_fenv_inline(int) fesetexcept(int __excepts)
{
    return feraiseexcept(__excepts);
}

__declare_fenv_inline(int) fetestexcept(int __excepts)
{
    return (int)(__hexagon_get_usr() & (unsigned long)(__excepts & FE_ALL_EXCEPT));
}

__declare_fenv_inline(int) fegetexceptflag(fexcept_t *__flagp, int __excepts)
{
    *__flagp = (fexcept_t)(__hexagon_get_usr() & (unsigned long)(__excepts & FE_ALL_EXCEPT));
    return 0;
}

__declare_fenv_inline(int) fesetexceptflag(const fexcept_t *__flagp, int __excepts)
{
    unsigned long __mask = (unsigned long)(__excepts & FE_ALL_EXCEPT);
    unsigned long __usr = __hexagon_get_usr();
    __hexagon_set_usr((__usr & ~__mask) | ((unsigned long)*__flagp & __mask));
    return 0;
}

__declare_fenv_inline(int) feholdexcept(fenv_t *__envp)
{
    unsigned long __usr = __hexagon_get_usr();
    *__envp = (fenv_t)(__usr & _FE_FP_MASK);
    /* Clear exception flags and exception enables; preserve rounding mode. */
    __hexagon_set_usr(
        __usr
        & ~(unsigned long)(FE_ALL_EXCEPT | ((unsigned long)FE_ALL_EXCEPT << _FE_ENABLE_SHIFT)));
    return 0;
}

__declare_fenv_inline(int) feupdateenv(const fenv_t *__envp)
{
    unsigned long __flags = __hexagon_get_usr() & (unsigned long)FE_ALL_EXCEPT;
    fesetenv(__envp);
    feraiseexcept((int)__flags);
    return 0;
}

__declare_fenv_inline(int) feenableexcept(int __excepts)
{
    unsigned long __usr = __hexagon_get_usr();
    unsigned long __old = (__usr >> _FE_ENABLE_SHIFT) & (unsigned long)FE_ALL_EXCEPT;
    unsigned long __new = __old | (unsigned long)(__excepts & FE_ALL_EXCEPT);
    __hexagon_set_usr((__usr & ~((unsigned long)FE_ALL_EXCEPT << _FE_ENABLE_SHIFT))
                      | (__new << _FE_ENABLE_SHIFT));
    return (int)__old;
}

__declare_fenv_inline(int) fedisableexcept(int __excepts)
{
    unsigned long __usr = __hexagon_get_usr();
    unsigned long __old = (__usr >> _FE_ENABLE_SHIFT) & (unsigned long)FE_ALL_EXCEPT;
    unsigned long __new = __old & ~(unsigned long)(__excepts & FE_ALL_EXCEPT);
    __hexagon_set_usr((__usr & ~((unsigned long)FE_ALL_EXCEPT << _FE_ENABLE_SHIFT))
                      | (__new << _FE_ENABLE_SHIFT));
    return (int)__old;
}

__declare_fenv_inline(int) fegetexcept(void)
{
    return (int)((__hexagon_get_usr() >> _FE_ENABLE_SHIFT) & (unsigned long)FE_ALL_EXCEPT);
}
