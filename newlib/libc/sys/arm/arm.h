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

#include "arm-acle-compat.h"

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

/* A and R profiles (and legacy Arm).
	Current Program Status Register (CPSR)
	M[4:0]		Mode bits. M[4] is always 1 for 32-bit modes.
	T[5]			1: Thumb, 0: ARM instruction set
	F[6]			1: disables FIQ
	I[7]			1: disables IRQ
	A[8]			1: disables imprecise aborts
	E[9]			0: Little-endian, 1: Big-endian
	J[24]			1: Jazelle instruction set
 */
#define CPSR_M_USR			0x00	/* User mode.  */
#define CPSR_M_FIQ			0x01	/* Fast Interrupt mode.  */
#define CPSR_M_IRQ			0x02	/* Interrupt mode.  */
#define CPSR_M_SVR			0x03	/* Supervisor mode.  */
#define CPSR_M_MON			0x06	/* Monitor mode.  */
#define CPSR_M_ABT			0x07	/* Abort mode.  */
#define CPSR_M_HYP			0x0A	/* Hypervisor mode.  */
#define CPSR_M_UND			0x0B	/* Undefined mode.  */
#define CPSR_M_SYS			0x0F	/* System mode.  */
#define CPSR_M_32BIT		0x10	/* 32-bit mode.  */
#define CPSR_T_BIT			0x20	/* Thumb bit.  */
#define CPSR_F_MASK			0x40	/* FIQ bit.  */
#define CPSR_I_MASK			0x80	/* IRQ bit.  */

#define CPSR_M_MASK			0x0F	/* Mode mask except M[4].  */

#endif /* _LIBGLOSS_ARM_H */
