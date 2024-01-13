/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
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
feclearexcept(int excepts)
{
	fexcept_t fcsr;

	excepts &= FE_ALL_EXCEPT;
	__cfc1(fcsr);
	fcsr &= ~(excepts | (excepts << _FCSR_CAUSE_SHIFT));
	__ctc1(fcsr);

	return (0);
}

__declare_fenv_inline(int)
fegetexceptflag(fexcept_t *flagp, int excepts)
{
	fexcept_t fcsr;

	excepts &= FE_ALL_EXCEPT;
	__cfc1(fcsr);
	*flagp = fcsr & excepts;

	return (0);
}

__declare_fenv_inline(int)
fesetexceptflag(const fexcept_t *flagp, int excepts)
{
	fexcept_t fcsr;

	excepts &= FE_ALL_EXCEPT;
	__cfc1(fcsr);
	fcsr &= ~excepts;
	fcsr |= *flagp & excepts;
	__ctc1(fcsr);

	return (0);
}

__declare_fenv_inline(int)
feraiseexcept(int excepts)
{
	fexcept_t fcsr;

	excepts &= FE_ALL_EXCEPT;
	__cfc1(fcsr);
	fcsr |= excepts | (excepts << _FCSR_CAUSE_SHIFT);
	__ctc1(fcsr);

	return (0);
}

__declare_fenv_inline(int)
fesetexcept(int excepts)
{
	fexcept_t fcsr;

	excepts &= FE_ALL_EXCEPT;
	__cfc1(fcsr);
	fcsr |= excepts | (excepts << _FCSR_CAUSE_SHIFT);
	__ctc1(fcsr);

	return (0);
}

__declare_fenv_inline(int)
fetestexcept(int excepts)
{
	fexcept_t fcsr;

	excepts &= FE_ALL_EXCEPT;
	__cfc1(fcsr);

	return (fcsr & excepts);
}

__declare_fenv_inline(int)
fegetround(void)
{
	fexcept_t fcsr;

	__cfc1(fcsr);

	return (fcsr & _ROUND_MASK);
}

__declare_fenv_inline(int)
fesetround(int rounding_mode)
{
	fexcept_t fcsr;

	if (rounding_mode & ~_ROUND_MASK)
		return (-1);

	__cfc1(fcsr);
	fcsr &= ~_ROUND_MASK;
	fcsr |= rounding_mode;
	__ctc1(fcsr);

	return (0);
}

__declare_fenv_inline(int)
fegetenv(fenv_t *envp)
{

	__cfc1(*envp);

	return (0);
}

__declare_fenv_inline(int)
feholdexcept(fenv_t *envp)
{
	fexcept_t fcsr;

	__cfc1(fcsr);
	*envp = fcsr;
	fcsr &= ~(FE_ALL_EXCEPT | _FCSR_ENABLE_MASK);
	__ctc1(fcsr);

	return (0);
}

__declare_fenv_inline(int)
fesetenv(const fenv_t *envp)
{

	__ctc1(*envp);

	return (0);
}

__declare_fenv_inline(int)
feupdateenv(const fenv_t *envp)
{
	fexcept_t fcsr;

	__cfc1(fcsr);
	fesetenv(envp);
	feraiseexcept(fcsr);

	return (0);
}

#if __BSD_VISIBLE

/* We currently provide no external definitions of the functions below. */

__declare_fenv_inline(int)
feenableexcept(int __mask)
{
	fenv_t __old_fcsr, __new_fcsr;

	__cfc1(__old_fcsr);
	__new_fcsr = __old_fcsr | (__mask & FE_ALL_EXCEPT) << _FCSR_ENABLE_SHIFT;
	__ctc1(__new_fcsr);

	return ((__old_fcsr >> _FCSR_ENABLE_SHIFT) & FE_ALL_EXCEPT);
}

__declare_fenv_inline(int)
fedisableexcept(int __mask)
{
	fenv_t __old_fcsr, __new_fcsr;

	__cfc1(__old_fcsr);
	__new_fcsr = __old_fcsr & ~((__mask & FE_ALL_EXCEPT) << _FCSR_ENABLE_SHIFT);
	__ctc1(__new_fcsr);

	return ((__old_fcsr >> _FCSR_ENABLE_SHIFT) & FE_ALL_EXCEPT);
}

__declare_fenv_inline(int)
fegetexcept(void)
{
	fexcept_t fcsr;

	__cfc1(fcsr);

	return ((fcsr & _FCSR_ENABLE_MASK) >> _FCSR_ENABLE_SHIFT);
}

#endif /* __BSD_VISIBLE */
