/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 2004-2005 David Schultz <das@FreeBSD.ORG>
 * Copyright (c) 2013 Andrew Turner <andrew@FreeBSD.ORG>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#define _SH_FPU_MASK_SHIFT 5
#define _SH_FPU_ROUND_MASK     3
#define _sh_get_fpscr(fpscr) __asm__ volatile ("sts fpscr,%0" : "=r" (fpscr))
#define _sh_set_fpscr(fpscr) __asm__ volatile ("lds %0,fpscr" : : "r" (fpscr))

__declare_fenv_inline(int) feclearexcept(int excepts)
{
	fexcept_t fpscr;

	_sh_get_fpscr(fpscr);
	fpscr &= ~excepts;
	_sh_set_fpscr(fpscr);
	return (0);
}

__declare_fenv_inline(int) fedisableexcept(int __mask)
{
	fenv_t __old_fpsr, __new_fpsr;

	_sh_get_fpscr(__old_fpsr);
	__new_fpsr = __old_fpsr &
	    ~((__mask & FE_ALL_EXCEPT) << _SH_FPU_MASK_SHIFT);
	_sh_set_fpscr(__new_fpsr);
	return ((__old_fpsr >> _SH_FPU_MASK_SHIFT) & FE_ALL_EXCEPT);
}

__declare_fenv_inline(int) feenableexcept(int __mask)
{
        fenv_t __old_fpsr, __new_fpsr, __check_fpsr;

	_sh_get_fpscr(__old_fpsr);
	__new_fpsr = __old_fpsr |
	    ((__mask & FE_ALL_EXCEPT) << _SH_FPU_MASK_SHIFT);
	_sh_set_fpscr(__new_fpsr);
        _sh_get_fpscr(__check_fpsr);
        if (__check_fpsr != __new_fpsr)
                return -1;
	return ((__old_fpsr >> _SH_FPU_MASK_SHIFT) & FE_ALL_EXCEPT);
}

__declare_fenv_inline(int) fegetenv(fenv_t *envp)
{
        fenv_t __env;
	_sh_get_fpscr(__env);
        *envp = __env;
	return (0);
}

__declare_fenv_inline(int) fegetexcept(void)
{
	fenv_t fpscr;

	_sh_get_fpscr(fpscr);
	return (fpscr & FE_ALL_EXCEPT);
}

__declare_fenv_inline(int) fegetexceptflag(fexcept_t *flagp, int excepts)
{
	fexcept_t fpscr;

	_sh_get_fpscr(fpscr);
        *flagp = fpscr & excepts;
	return (0);
}

__declare_fenv_inline(int) fegetround(void)
{
	fenv_t fpscr;

	_sh_get_fpscr(fpscr);
	return (fpscr & _SH_FPU_ROUND_MASK);
}

__declare_fenv_inline(int) feholdexcept(fenv_t *envp)
{
	fenv_t __env;

	_sh_get_fpscr(__env);
	*envp = __env;
	__env &= ~(FE_ALL_EXCEPT);
	_sh_set_fpscr(__env);
	return (0);
}

__declare_fenv_inline(int) fesetenv(const fenv_t *envp)
{

	_sh_set_fpscr(*envp);
	return (0);
}

__declare_fenv_inline(int) fesetexceptflag(const fexcept_t *flagp, int excepts)
{
	fexcept_t fpscr;

	_sh_get_fpscr(fpscr);
	fpscr &= ~excepts;
	fpscr |= *flagp & excepts;
	_sh_set_fpscr(fpscr);
	return (0);
}

__declare_fenv_inline(int) feraiseexcept(int excepts)
{
	fexcept_t __ex = excepts;

	fesetexceptflag(&__ex, excepts);
	return (0);
}

__declare_fenv_inline(int) fesetexcept(int excepts)
{
        return feraiseexcept(excepts);
}

__declare_fenv_inline(int) fesetround(int round)
{
	fenv_t fpscr;

	_sh_get_fpscr(fpscr);
	fpscr &= ~(_SH_FPU_ROUND_MASK);
	fpscr |= round;
	_sh_set_fpscr(fpscr);
	return (0);
}

__declare_fenv_inline(int) fetestexcept(int excepts)
{
	fexcept_t fpscr;

	_sh_get_fpscr(fpscr);
	return (fpscr & excepts);
}

__declare_fenv_inline(int) feupdateenv(const fenv_t *envp)
{
	fexcept_t fpscr;

	_sh_get_fpscr(fpscr);
	_sh_set_fpscr(*envp);
	feraiseexcept(fpscr & FE_ALL_EXCEPT);
	return (0);
}

#undef _SH_FPU_ROUND_MASK
#undef _SH_FPU_MASK_SHIFT
#undef _sh_get_fpscr
#undef _sh_set_fpscr
