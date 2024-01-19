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


#ifndef _SYS_FENV_H
#define _SYS_FENV_H

#include <sys/cdefs.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long fenv_t;
typedef unsigned long fexcept_t;

#include <machine/core-isa.h>

#if XCHAL_HAVE_FP || XCHAL_HAVE_DFP

#define FE_DIVBYZERO   0x08
#define FE_INEXACT     0x01
#define FE_INVALID     0x10
#define FE_OVERFLOW    0x04
#define FE_UNDERFLOW   0x02

#define FE_ALL_EXCEPT \
  (FE_DIVBYZERO  |		      \
   FE_INEXACT    |		      \
   FE_INVALID    |		      \
   FE_OVERFLOW   |		      \
   FE_UNDERFLOW)

#define FE_DOWNWARD   0x3
#define FE_TONEAREST  0x0
#define FE_TOWARDZERO 0x1
#define FE_UPWARD     0x2

#define _FE_EXCEPTION_FLAGS_OFFSET 7
#define _FE_EXCEPTION_FLAG_MASK (FE_ALL_EXCEPT << _FE_EXCEPTION_FLAGS_OFFSET)
#define _FE_EXCEPTION_ENABLE_OFFSET 2
#define _FE_EXCEPTION_ENABLE_MASK (FE_ALL_EXCEPT << _FE_EXCEPTION_ENABLE_OFFSET)
#define _FE_ROUND_MODE_OFFSET 0
#define _FE_ROUND_MODE_MASK (0x3 << _FE_ROUND_MODE_OFFSET)
#define _FE_FLOATING_ENV_MASK (_FE_EXCEPTION_FLAG_MASK | _FE_EXCEPTION_ENABLE_MASK | _FE_ROUND_MODE_MASK)

#endif

#if !defined(__declare_fenv_inline) && defined(__declare_extern_inline)
#define	__declare_fenv_inline(type) __declare_extern_inline(type)
#endif

#ifdef __declare_fenv_inline
#if XCHAL_HAVE_FP || XCHAL_HAVE_DFP
#include <machine/fenv-fp.h>
#else
#include <machine/fenv-softfloat.h>
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif
