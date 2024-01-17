/*-
 * Copyright (c) 2004-2005 David Schultz <das@FreeBSD.ORG>
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


__declare_fenv_inline(int)
feclearexcept(int __excepts)
{
	fexcept_t __r;

	__mrs_fpsr(__r);
	__r &= ~__excepts;
	__msr_fpsr(__r);
	return (0);
}

__declare_fenv_inline(int)
fegetexceptflag(fexcept_t *__flagp, int __excepts)
{
	fexcept_t __r;

	__mrs_fpsr(__r);
	*__flagp = __r & __excepts;
	return (0);
}

__declare_fenv_inline(int)
fesetexceptflag(const fexcept_t *__flagp, int __excepts)
{
	fexcept_t __r;

	__mrs_fpsr(__r);
	__r &= ~__excepts;
	__r |= *__flagp & __excepts;
	__msr_fpsr(__r);
	return (0);
}

__declare_fenv_inline(int)
feraiseexcept(int __excepts)
{
	fexcept_t __r;

	__mrs_fpsr(__r);
	__r |= __excepts;
	__msr_fpsr(__r);
	return (0);
}

__declare_fenv_inline(int)
fesetexcept(int excepts)
{
        return feraiseexcept(excepts);
}

__declare_fenv_inline(int)
fetestexcept(int __excepts)
{
	fexcept_t __r;

	__mrs_fpsr(__r);
	return (__r & __excepts);
}

__declare_fenv_inline(int)
fegetround(void)
{
	fenv_t __r;

	__mrs_fpcr(__r);
	return ((__r >> _ROUND_SHIFT) & _ROUND_MASK);
}

__declare_fenv_inline(int)
fesetround(int __round)
{
	fenv_t __r;

	if (__round & ~_ROUND_MASK)
		return (-1);
	__mrs_fpcr(__r);
	__r &= ~(_ROUND_MASK << _ROUND_SHIFT);
	__r |= __round << _ROUND_SHIFT;
	__msr_fpcr(__r);
	return (0);
}

__declare_fenv_inline(int)
fegetenv(fenv_t *__envp)
{
	fenv_t __r;

	__mrs_fpcr(__r);
	*__envp = __r & _ENABLE_MASK;

	__mrs_fpsr(__r);
	*__envp |= __r & (FE_ALL_EXCEPT | (_ROUND_MASK << _ROUND_SHIFT));

	return (0);
}

__declare_fenv_inline(int)
feholdexcept(fenv_t *__envp)
{
	fenv_t __r;

	__mrs_fpcr(__r);
	*__envp = __r & _ENABLE_MASK;
	__r &= ~(_ENABLE_MASK);
	__msr_fpcr(__r);

	__mrs_fpsr(__r);
	*__envp |= __r & (FE_ALL_EXCEPT | (_ROUND_MASK << _ROUND_SHIFT));
	__r &= ~(_ENABLE_MASK);
	__msr_fpsr(__r);
	return (0);
}

__declare_fenv_inline(int)
fesetenv(const fenv_t *__envp)
{

	__msr_fpcr((*__envp) & _ENABLE_MASK);
	__msr_fpsr((*__envp) & (FE_ALL_EXCEPT | (_ROUND_MASK << _ROUND_SHIFT)));
	return (0);
}

__declare_fenv_inline(int)
feupdateenv(const fenv_t *__envp)
{
	fexcept_t __r;

	__mrs_fpsr(__r);
	fesetenv(__envp);
	feraiseexcept(__r & FE_ALL_EXCEPT);
	return (0);
}

__declare_fenv_inline(int)
feenableexcept(int __mask)
{
        fenv_t __old_r, __new_r, __test_r;

	__mrs_fpcr(__old_r);
	__new_r = __old_r | ((__mask & FE_ALL_EXCEPT) << _FPUSW_SHIFT);
	__msr_fpcr(__new_r);
        __asm ("mrs %0, fpcr" : "=r" (__test_r));
        if (__new_r != __test_r)
            return -1;
	return ((__old_r >> _FPUSW_SHIFT) & FE_ALL_EXCEPT);
}

__declare_fenv_inline(int)
fedisableexcept(int __mask)
{
	fenv_t __old_r, __new_r;

	__mrs_fpcr(__old_r);
	__new_r = __old_r & ~((__mask & FE_ALL_EXCEPT) << _FPUSW_SHIFT);
	__msr_fpcr(__new_r);
	return ((__old_r >> _FPUSW_SHIFT) & FE_ALL_EXCEPT);
}

__declare_fenv_inline(int)
fegetexcept(void)
{
	fenv_t __r;

	__mrs_fpcr(__r);
	return ((__r & _ENABLE_MASK) >> _FPUSW_SHIFT);
}
