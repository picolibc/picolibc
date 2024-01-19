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
	union __fpscr __r;

	if (excepts & FE_INVALID)
		excepts |= FE_ALL_INVALID;
	__mffs(&__r);
	__r.__bits.__reg &= ~excepts;
	__mtfsf(__r);
	return (0);
}


__declare_fenv_inline(int)
fegetexceptflag(fexcept_t *flagp, int excepts)
{
	union __fpscr __r;

	__mffs(&__r);
	*flagp = __r.__bits.__reg & excepts;
	return (0);
}

__declare_fenv_inline(int)
fesetexceptflag(const fexcept_t *flagp, int excepts)
{
	union __fpscr __r;

	if (excepts & FE_INVALID)
		excepts |= FE_ALL_EXCEPT;
	__mffs(&__r);
	__r.__bits.__reg &= ~excepts;
	__r.__bits.__reg |= *flagp & excepts;
	__mtfsf(__r);
	return (0);
}

__declare_fenv_inline(int)
feraiseexcept(int excepts)
{
	union __fpscr __r;

	if (excepts & FE_INVALID)
		excepts |= FE_VXSOFT;
	__mffs(&__r);
	__r.__bits.__reg |= excepts;
	__mtfsf(__r);
	return (0);
}

__declare_fenv_inline(int)
fesetexcept(int excepts)
{
        return feraiseexcept(excepts);
}

__declare_fenv_inline(int)
fetestexcept(int excepts)
{
	union __fpscr __r;

	__mffs(&__r);
	return (__r.__bits.__reg & excepts);
}

__declare_fenv_inline(int)
fegetround(void)
{
	union __fpscr __r;

	__mffs(&__r);
	return (__r.__bits.__reg & _ROUND_MASK);
}

__declare_fenv_inline(int)
fesetround(int rounding_mode)
{
	union __fpscr __r;

	if (rounding_mode & ~_ROUND_MASK)
		return (-1);
	__mffs(&__r);
	__r.__bits.__reg &= ~_ROUND_MASK;
	__r.__bits.__reg |= rounding_mode;
	__mtfsf(__r);
	return (0);
}

__declare_fenv_inline(int)
fegetenv(fenv_t *envp)
{
	union __fpscr __r;

	__mffs(&__r);
	*envp = __r.__bits.__reg;
	return (0);
}

__declare_fenv_inline(int)
feholdexcept(fenv_t *envp)
{
	union __fpscr __r;

	__mffs(&__r);
	*envp = __r.__bits.__reg;
	__r.__bits.__reg &= ~(FE_ALL_EXCEPT | _ENABLE_MASK);
	__mtfsf(__r);
	return (0);
}

__declare_fenv_inline(int)
fesetenv(const fenv_t *envp)
{
	union __fpscr __r;

	__r.__bits.__reg = *envp;
	__mtfsf(__r);
	return (0);
}

__declare_fenv_inline(int)
feupdateenv(const fenv_t *envp)
{
	union __fpscr __r;

	__mffs(&__r);
	__r.__bits.__reg &= FE_ALL_EXCEPT;
	__r.__bits.__reg |= *envp;
	__mtfsf(__r);
	return (0);
}

#if __BSD_VISIBLE

__declare_fenv_inline(int)
feenableexcept(int __mask)
{
	union __fpscr __r;
	fenv_t __oldmask;

	__mffs(&__r);
	__oldmask = __r.__bits.__reg;
	__r.__bits.__reg |= (__mask & FE_ALL_EXCEPT) >> _FPUSW_SHIFT;
	__mtfsf(__r);
	return ((__oldmask & _ENABLE_MASK) << _FPUSW_SHIFT);
}

__declare_fenv_inline(int)
fedisableexcept(int __mask)
{
	union __fpscr __r;
	fenv_t __oldmask;

	__mffs(&__r);
	__oldmask = __r.__bits.__reg;
	__r.__bits.__reg &= ~((__mask & FE_ALL_EXCEPT) >> _FPUSW_SHIFT);
	__mtfsf(__r);
	return ((__oldmask & _ENABLE_MASK) << _FPUSW_SHIFT);
}

__declare_fenv_inline(int)
fegetexcept(void)
{
	union __fpscr __r;

	__mffs(&__r);
	return ((__r.__bits.__reg & _ENABLE_MASK) << _FPUSW_SHIFT);
}

#endif /* __BSD_VISIBLE */
