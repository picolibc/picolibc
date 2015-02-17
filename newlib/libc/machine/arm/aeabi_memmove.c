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
   aeabi_memmove for the targets with FP register.  Instead, versions
   for these specific targets are written in assembler (in
   aeabi_memmove-soft.S).  */

/* NOTE: This ifdef MUST match the one in aeabi_memmove-soft.S.  */
#if !defined (__SOFTFP__)

/* Defined in aeabi_memmove-soft.S.  */

#else
/* Support the alias for the __aeabi_memmove which may
   assume memory alignment.  */
void __aeabi_memmove4 (void *dest, const void *source, size_t n)
	_ATTRIBUTE ((alias ("__aeabi_memmove")));

void __aeabi_memmove8 (void *dest, const void *source, size_t n)
	_ATTRIBUTE ((alias ("__aeabi_memmove")));

/* Support the routine __aeabi_memmove.  Can't alias to memmove
   because it's not defined in the same translation unit.  */
void __aeabi_memmove (void *dest, const void *source, size_t n)
{
  extern void memmove (void *dest, const void *source, size_t n);
  memmove (dest, source, n);
}
#endif
