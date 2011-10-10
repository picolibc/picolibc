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

/* __thumb2__ stands for thumb on armva7(A/R/M/EM) architectures,
   __ARM_ARCH_6M__ stands for armv6-M(thumb only) architecture,
   __ARM_ARCH_7M__ stands for armv7-M(thumb only) architecture.
   __ARM_ARCH_7EM__ stands for armv7e-M(thumb only) architecture.
   There are some macro combinations used many times in libgloss/arm,
   like (__thumb2__ || (__thumb__ && __ARM_ARCH_6M__)), so factor
   it out and use THUMB_V7_V6M instead, which stands for thumb on
   v6-m/v7 arch as the combination does.  */
#if defined(__thumb2__) || (defined(__thumb__) && defined(__ARM_ARCH_6M__))
# define THUMB_V7_V6M
#endif

/* The (__ARM_ARCH_7EM__ || __ARM_ARCH_7M__ || __ARM_ARCH_6M__) combination
   stands for cortex-M profile architectures, which don't support ARM state.
   Factor it out and use THUMB_V7M_V6M instead.  */
#if defined(__ARM_ARCH_7M__)     \
    || defined(__ARM_ARCH_7EM__) \
    || defined(__ARM_ARCH_6M__)
# define THUMB_V7M_V6M
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
