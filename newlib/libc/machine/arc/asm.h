#ifndef ARC_NEWLIB_ASM_H
#define ARC_NEWLIB_ASM_H

/*
   Copyright (c) 2015, Synopsys, Inc. All rights reserved.

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


#define _ENTRY(name) \
	.text ` .balign 4 ` .globl name ` name:
#define FUNC(name)         .type name,@function
#define ENDFUNC0(name) .Lfe_##name: .size name,.Lfe_##name-name
#define ENDFUNC(name) ENDFUNC0 (name)
#define ENTRY(name) _ENTRY (name) ` FUNC (name)

#define add_l	add
#define bcc_l	bcc
#define bclr_l	bclr
#define beq_l	beq
#define bic_l	bic
#define b_l	b
#define bne_l	bne
#define breq_l	breq
#define brne_l	brne
#define j_l	j
#define ldb_l	ldb
#define ld_l	ld
#define mov_l	mov
#define or_l	or
#define st_l	st
#define stb_l	stb
#define sub_l	sub
#define tst_l	tst
#define extb_l	extb

#define bcc_s	bhs_s

/* Compatibility with older ARC GCC, that doesn't provide some of the
   preprocessor defines used by newlib for ARC.  */
#if defined (__Xbarrel_shifter) && !defined (__ARC_BARREL_SHIFTER__)
#define __ARC_BARREL_SHIFTER__ 1
#endif

#if defined (__EM__) && !defined (__ARCEM__)
#define __ARCEM__ 1
#endif

#if defined (__HS__) && !defined (__ARCHS__)
#define __ARCHS__ 1
#endif

#if defined (__LL64__) && !defined (__ARC_LL64__)
#define __ARC_LL64__ 1
#endif

#endif /* ARC_NEWLIB_ASM_H */
