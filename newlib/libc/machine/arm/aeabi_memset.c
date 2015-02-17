/*
 * Copyright (c) 2015 ARM Ltd
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

/* According to the run-time ABI for the ARM Architecture, this
   function is allowed to corrupt only the integer core register
   permitted to be corrupted by the [AAPCS] (r0-r3, ip, lr, and
   CPSR).

   Therefore, we can't just simply use alias to support the function
   aeabi_memset for the targets with FP register.  Instead, versions
   for these specific targets are written in assembler (in
   aeabi_memset-soft.S).  */

/* NOTE: This ifdef MUST match the one in aeabi_memset-soft.S.  */
#if !defined (__SOFTFP__)

/* Defined in aeabi_memset-soft.S.  */

#else
/* Support the alias for the __aeabi_memset which may
   assume memory alignment.  */
void __aeabi_memset4 (void *dest, size_t n, int c)
	_ATTRIBUTE ((alias ("__aeabi_memset")));

void __aeabi_memset8 (void *dest, size_t n, int c)
	_ATTRIBUTE ((alias ("__aeabi_memset")));

/* Support the routine __aeabi_memset.  Can't alias to memset
   because it's not defined in the same translation unit.  */
void __aeabi_memset (void *dest, size_t n, int c)
{
  /*Note that relative to ANSI memset, __aeabi_memset hase the order
    of its second and third arguments reversed.  */
  extern void memset (void *dest, int c, size_t n);
  memset (dest, c, n);
}
#endif
