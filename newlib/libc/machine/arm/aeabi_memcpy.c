/*
 * Copyright (c) 2014 ARM Ltd
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
 * 3. The name of the company may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ARM LTD ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ARM LTD BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stddef.h>
#include <_ansi.h>

/* According to the Run-time ABI for the ARM Architecture.  This
   function is allowed to corrupt only the integer core register
   permitted to be corrupted by the [AAPCS] (r0-r3, ip, lr, and
   CPSR).

   The FP registers are used in memcpy for target __ARM_ARCH_7A.
   Therefore, we can't just simply use alias to support the function
   aeabi_memcpy for target __ARM_ARCH_7A.  Instead, we choose the
   previous versions of memcpy to suppport it as an alternative.  */

/* NOTE: This ifdef MUST match the one in aeabi_memcpy-armv7a.S.  */
#if defined (__ARM_ARCH_7A__) && defined (__ARM_FEATURE_UNALIGNED) && \
	(defined (__ARM_NEON__) || !defined (__SOFTFP__))

/* Defined in aeabi_memcpy-armv7a.S.  */

#else
/* Support the alias for the __aeabi_memcpy which may
   assume memory alignment.  */
void __aeabi_memcpy4 (void *dest, const void *source, size_t n)
	_ATTRIBUTE ((alias ("__aeabi_memcpy")));

void __aeabi_memcpy8 (void *dest, const void *source, size_t n)
	_ATTRIBUTE ((alias ("__aeabi_memcpy")));

/* Support the routine __aeabi_memcpy.  Can't alias to memcpy
   because it's not defined in the same translation unit.  */
void __aeabi_memcpy (void *dest, const void *source, size_t n)
{
  extern void memcpy (void *dest, const void *source, size_t n);
  memcpy (dest, source, n);
}
#endif
