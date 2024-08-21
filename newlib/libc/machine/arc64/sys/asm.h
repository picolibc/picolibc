/*
   Copyright (c) 2024, Synopsys, Inc. All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   1) Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

   2) Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

   3) Neither the name of the Synopsys, Inc., nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _SYS_ASM_H
#define _SYS_ASM_H

/*
 * Macros to handle different pointer/register sizes for 32/64-bit code
 */
#if defined (__ARC64_ARCH32__)
# define ST64		std
# define LD64		ldd
# define MOVP		mov
# define LSRP		lsr
# define ADDP		add
# define SUBP		sub
# define REG_SZ		4
# define REG_ST		st
# define REG_LD		ld
#elif defined (__ARC64_ARCH64__)
# define ST64		stl
# define LD64		ldl
# define MOVP		movl
# define LSRP		lsrl
# define ADDP		addl
# define SUBP		subl
# define REG_SZ		8
# define REG_ST		stl
# define REG_LD		ldl
#else
# error Please use either 32-bit or 64-bit version of arc64 compiler
#endif

# define NULL_32DT_1	0x01010101
# define NULL_32DT_2	0x80808080

#define _ENTRY(name)	.text ` .balign 4 ` .globl name ` name:
#define FUNC(name)	.type name,@function
#define ENDFUNC0(name)	.Lfe_##name: .size name,.Lfe_##name-name
#define ENDFUNC(name)	ENDFUNC0 (name)
#define ENTRY(name)	_ENTRY (name) ` FUNC (name)

#endif /* _SYS_ASM_H */
