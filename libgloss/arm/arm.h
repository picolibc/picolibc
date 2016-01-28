/*
 * Copyright (c) 2011 ARM Ltd
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

#ifndef _LIBGLOSS_ARM_H
#define _LIBGLOSS_ARM_H

#include "acle-compat.h"

/* Checking for targets supporting only Thumb instructions (eg. ARMv6-M) or
   supporting Thumb-2 instructions, whether ARM instructions are available or
   not, is done many times in libgloss/arm.  So factor it out and use
   PREFER_THUMB instead.  */
#if __thumb2__ || (__thumb__ && !__ARM_ARCH_ISA_ARM)
# define PREFER_THUMB
#endif

/* Processor only capable of executing Thumb-1 instructions.  */
#if __ARM_ARCH_ISA_THUMB == 1 && !__ARM_ARCH_ISA_ARM
# define THUMB1_ONLY
#endif

/* M profile architectures.  This is a different set of architectures than
   those not having ARM ISA because it does not contain ARMv7.  This macro is
   necessary to test which architectures use bkpt as semihosting interface from
   architectures using svc.  */
#if !__ARM_ARCH_ISA_ARM && !__ARM_ARCH_7__
# define THUMB_VXM
#endif

/* Defined if this target supports the BLX Rm instruction.  */
#if  !defined(__ARM_ARCH_2__)   \
  && !defined(__ARM_ARCH_3__)	\
  && !defined(__ARM_ARCH_3M__)	\
  && !defined(__ARM_ARCH_4__)	\
  && !defined(__ARM_ARCH_4T__)
# define HAVE_CALL_INDIRECT
#endif

#endif /* _LIBGLOSS_ARM_H */
