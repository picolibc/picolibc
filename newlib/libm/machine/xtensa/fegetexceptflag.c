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

#include <xtensa/config/core-isa.h>

#if XCHAL_HAVE_FP || XCHAL_HAVE_DFP

#include <fenv.h>

int fegetexceptflag(fexcept_t *flagp, int excepts)
{
  unsigned int fsr;
  if (excepts & ~FE_ALL_EXCEPT)
    return -1;
  asm ("rur.fsr %0" : "=a"(fsr));
  fsr >>= _FE_EXCEPTION_FLAGS_OFFSET;
  excepts &= fsr;
  *flagp = excepts;

  return 0;
}


int fesetexceptflag(const fexcept_t *flagp, int excepts)
{
  if (excepts & ~FE_ALL_EXCEPT)
    return -1;

  unsigned int fsr;

  asm ("rur.fsr %0" : "=a"(fsr));

  fsr &= ~(excepts << _FE_EXCEPTION_FLAGS_OFFSET);
  fsr |= ((*flagp & excepts) << _FE_EXCEPTION_FLAGS_OFFSET);
  asm ("wur.fsr %0" : : "a"(fsr));
  return 0;
}

#endif
