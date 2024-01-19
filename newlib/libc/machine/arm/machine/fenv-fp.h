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

#define	_vmrs_fpscr(__r)	__asm __volatile("vmrs %0, fpscr" : "=&r"(__r))
#define	_vmsr_fpscr(__r)	__asm __volatile("vmsr fpscr, %0" : : "r"(__r))
#define	_FPU_MASK_SHIFT	8
#define	_ROUND_MASK	(FE_TONEAREST | FE_DOWNWARD | \
			 FE_UPWARD | FE_TOWARDZERO)

__declare_fenv_inline(int) feclearexcept(int excepts)
{
	fexcept_t __fpsr;

	_vmrs_fpscr(__fpsr);
	__fpsr &= ~excepts;
	_vmsr_fpscr(__fpsr);
	return (0);
}

__declare_fenv_inline(int) fedisableexcept(int __mask)
{
	fenv_t __old_fpsr, __new_fpsr;

	_vmrs_fpscr(__old_fpsr);
	__new_fpsr = __old_fpsr &
	    ~((__mask & FE_ALL_EXCEPT) << _FPU_MASK_SHIFT);
	_vmsr_fpscr(__new_fpsr);
	return ((__old_fpsr >> _FPU_MASK_SHIFT) & FE_ALL_EXCEPT);
}
__declare_fenv_inline(int) feenableexcept(int __mask)
{
        fenv_t __old_fpsr, __new_fpsr, __check_fpsr;

	_vmrs_fpscr(__old_fpsr);
	__new_fpsr = __old_fpsr |
	    ((__mask & FE_ALL_EXCEPT) << _FPU_MASK_SHIFT);
	_vmsr_fpscr(__new_fpsr);
        _vmrs_fpscr(__check_fpsr);
        if (__check_fpsr != __new_fpsr)
                return -1;
	return ((__old_fpsr >> _FPU_MASK_SHIFT) & FE_ALL_EXCEPT);
}

__declare_fenv_inline(int) fegetenv(fenv_t *envp)
{
	_vmrs_fpscr(*envp);
	return (0);
}

__declare_fenv_inline(int) fegetexcept(void)
{
	fenv_t __fpsr;

	_vmrs_fpscr(__fpsr);
	return (__fpsr & FE_ALL_EXCEPT);
}

__declare_fenv_inline(int) fegetexceptflag(fexcept_t *flagp, int excepts)
{
	fexcept_t __fpsr;

	_vmrs_fpscr(__fpsr);
        *flagp = __fpsr & excepts;
	return (0);
}

__declare_fenv_inline(int) fegetround(void)
{
	fenv_t __fpsr;

	_vmrs_fpscr(__fpsr);
	return (__fpsr & _ROUND_MASK);
}
__declare_fenv_inline(int) feholdexcept(fenv_t *envp)
{
	fenv_t __env;

	_vmrs_fpscr(__env);
	*envp = __env;
	__env &= ~(FE_ALL_EXCEPT);
	_vmsr_fpscr(__env);
	return (0);
}

__declare_fenv_inline(int) fesetenv(const fenv_t *envp)
{
	_vmsr_fpscr(*envp);
	return (0);
}

__declare_fenv_inline(int) fesetexceptflag(const fexcept_t *flagp, int excepts)
{
	fexcept_t __fpsr;

	_vmrs_fpscr(__fpsr);
	__fpsr &= ~excepts;
	__fpsr |= *flagp & excepts;
	_vmsr_fpscr(__fpsr);
	return (0);
}

__declare_fenv_inline(int) feraiseexcept(int excepts)
{
	fexcept_t __ex = excepts;

	return fesetexceptflag(&__ex, excepts);
}

__declare_fenv_inline(int) fesetexcept(int excepts)
{
        return feraiseexcept(excepts);
}

__declare_fenv_inline(int) fesetround(int round)
{
	fenv_t __fpsr;

	_vmrs_fpscr(__fpsr);
	__fpsr &= ~(_ROUND_MASK);
	__fpsr |= round;
	_vmsr_fpscr(__fpsr);
	return (0);
}

__declare_fenv_inline(int) fetestexcept(int excepts)
{
	fexcept_t __fpsr;

	_vmrs_fpscr(__fpsr);
	return (__fpsr & excepts);
}

__declare_fenv_inline(int) feupdateenv(const fenv_t *envp)
{
	fexcept_t __fpsr;

	_vmrs_fpscr(__fpsr);
	_vmsr_fpscr(*envp);
	feraiseexcept(__fpsr & FE_ALL_EXCEPT);
	return (0);
}
