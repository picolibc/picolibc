/* Copyright (c) 2009, 2010, 2011, 2012 ARM Ltd.  All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:
 1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
 3. The name of the company may not be used to endorse or promote
    products derived from this software without specific prior written
    permission.

 THIS SOFTWARE IS PROVIDED BY ARM LTD ``AS IS'' AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL ARM LTD BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#include <_ansi.h>

/* Now the SWI numbers and reason codes for RDI (Angel) monitors.  */
#define AngelSVC				0xF000
#define AngelSVCInsn				"hlt"
#define AngelSVCAsm				hlt

/* The reason codes:  */
#define AngelSVC_Reason_Open			0x01
#define AngelSVC_Reason_Close			0x02
#define AngelSVC_Reason_WriteC			0x03
#define AngelSVC_Reason_Write0			0x04
#define AngelSVC_Reason_Write			0x05
#define AngelSVC_Reason_Read			0x06
#define AngelSVC_Reason_ReadC			0x07
#define AngelSVC_Reason_IsError			0x08
#define AngelSVC_Reason_IsTTY			0x09
#define AngelSVC_Reason_Seek			0x0A
#define AngelSVC_Reason_FLen			0x0C
#define AngelSVC_Reason_TmpNam			0x0D
#define AngelSVC_Reason_Remove			0x0E
#define AngelSVC_Reason_Rename			0x0F
#define AngelSVC_Reason_Clock			0x10
#define AngelSVC_Reason_Time			0x11
#define AngelSVC_Reason_System			0x12
#define AngelSVC_Reason_Errno			0x13
#define AngelSVC_Reason_GetCmdLine		0x15
#define AngelSVC_Reason_HeapInfo		0x16
#define AngelSVC_Reason_EnterSVC		0x17
#define AngelSVC_Reason_ReportException 	0x18
#define AngelSVC_Reason_SyncCacheRange		0x19
#define AngelSVC_Reason_ReportExceptionExtended 0x20
#define AngelSVC_Reason_Elapsed			0x30
#define AngelSVC_Reason_TickFreq		0x31
#define ADP_Stopped_ApplicationExit		((2 << 16) + 38)
#define ADP_Stopped_RunTimeError		((2 << 16) + 35)

/* Semihosting feature magic numbers.  */
#define NUM_SHFB_MAGIC			4
#define SHFB_MAGIC_0			0x53
#define SHFB_MAGIC_1			0x48
#define SHFB_MAGIC_2			0x46
#define SHFB_MAGIC_3			0x42

/* Semihosting extensions.  */
#define SH_EXT_EXIT_EXTENDED_BITNUM	0x0
#define SH_EXT_STDOUT_STDERR_BITNUM	0x1

#if !defined (__ASSEMBLER__)
extern int _get_semihosting_exts (char*, int, int);
extern int _has_ext_exit_extended (void);
extern int _has_ext_stdout_stderr (void);
#endif

#if defined(ARM_RDI_MONITOR) && !defined(__ASSEMBLER__)

/* Type of each entry in a parameter block.  */
typedef long long param_block_t;

static inline long long
do_AngelSVC (int reason, param_block_t * arg)
{
  long long value;
  asm volatile ("mov w0, %w1; mov x1, %2; " AngelSVCInsn " %3; mov %0, x0"
		: "=r" (value) /* Outputs */
		: "r" (reason), "r" (arg), "n" (AngelSVC) /* Inputs */
		: "x0", "x1", "x2", "x3", "x17", "x30", "memory", "cc"
		  /* Clobbers x0 and x1, and lr if in supervisor mode */);
  return value;
}

#endif
