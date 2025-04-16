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

#ifndef _MACHINE_FENV_H_
#define _MACHINE_FENV_H_

#include <sys/cdefs.h>

_BEGIN_STD_C

typedef int fenv_t;
typedef int fexcept_t;

#if defined(__HAVE_68881__)

#define FE_INVALID      0x0080
#define FE_OVERFLOW     0x0040
#define FE_UNDERFLOW    0x0020
#define FE_DIVBYZERO    0x0010
#define FE_INEXACT      0x0008

#define	FE_ALL_EXCEPT	(FE_INVALID | FE_OVERFLOW | \
    FE_UNDERFLOW | FE_DIVBYZERO | FE_INEXACT)

#define FE_TONEAREST    0x0000
#define FE_TOWARDZERO   0x0001
#define FE_DOWNWARD     0x0002
#define FE_UPWARD       0x0003

#else
#define FE_TONEAREST    0x0000
#endif

#if !defined(__declare_fenv_inline) && defined(__declare_extern_inline)
#define	__declare_fenv_inline(type) __declare_extern_inline(type)
#endif

#ifdef __declare_fenv_inline
#if defined(__HAVE_68881__)
#include <machine/fenv-fp.h>
#else
#include <machine/fenv-softfloat.h>
#endif
#endif

_END_STD_C

#endif	/* _MACHINE_FENV_H_ */
