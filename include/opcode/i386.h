/* opcode/i386.h -- Intel 80386 opcode table
   Copyright 1989, 91, 92, 93, 94, 95, 96, 97, 98, 1999 Free Software Foundation.

This file is part of GAS, the GNU Assembler, and GDB, the GNU Debugger.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

/* The UnixWare assembler, and probably other AT&T derived ix86 Unix
   assemblers, generate floating point instructions with reversed
   source and destination registers in certain cases.  Unfortunately,
   gcc and possibly many other programs use this reversed syntax, so
   we're stuck with it.

   eg. `fsub %st(3),%st' results in st <- st - st(3) as expected, but
   `fsub %st,%st(3)' results in st(3) <- st - st(3), rather than
   the expected st(3) <- st(3) - st !

   This happens with all the non-commutative arithmetic floating point
   operations with two register operands, where the source register is
   %st, and destination register is %st(i).  Look for FloatDR below.  */

#ifndef UNIXWARE_COMPAT
/* Set non-zero for broken, compatible instructions.  Set to zero for
   non-broken opcodes at your peril.  gcc generates UnixWare
   compatible instructions.  */
#define UNIXWARE_COMPAT 1
#endif


static const template i386_optab[] = {

#define X None
#define ReverseModrm (ReverseRegRegmem|Modrm)
#define NoSuf (No_bSuf|No_wSuf|No_lSuf|No_sSuf|No_dSuf|No_xSuf)
#define b_Suf (No_wSuf|No_lSuf|No_sSuf|No_dSuf|No_xSuf)
#define w_Suf (No_bSuf|No_lSuf|No_sSuf|No_dSuf|No_xSuf)
#define l_Suf (No_bSuf|No_wSuf|No_sSuf|No_dSuf|No_xSuf)
#define d_Suf (No_bSuf|No_wSuf|No_sSuf|No_lSuf|No_xSuf)
#define x_Suf (No_bSuf|No_wSuf|No_sSuf|No_lSuf|No_dSuf)
#define bw_Suf (No_lSuf|No_sSuf|No_dSuf|No_xSuf)
#define bl_Suf (No_wSuf|No_sSuf|No_dSuf|No_xSuf)
#define wl_Suf (No_bSuf|No_sSuf|No_dSuf|No_xSuf)
#define sl_Suf (No_bSuf|No_wSuf|No_dSuf|No_xSuf)
#define sld_Suf (No_bSuf|No_wSuf|No_xSuf)
#define sldx_Suf (No_bSuf|No_wSuf)
#define bwl_Suf (No_sSuf|No_dSuf|No_xSuf)
#define bwld_Suf (No_sSuf|No_xSuf)
#define FP (NoSuf|IgnoreSize)
#define l_FP (l_Suf|IgnoreSize)
#define d_FP (d_Suf|IgnoreSize)
#define x_FP (x_Suf|IgnoreSize)
#define sl_FP (sl_Suf|IgnoreSize)
#define sld_FP (sld_Suf|IgnoreSize)
#define sldx_FP (sldx_Suf|IgnoreSize)
#if UNIXWARE_COMPAT
#define FloatDR FloatD
#else
#define FloatDR (FloatD|FloatR)
#endif

/* move instructions */
#define MOV_AX_DISP32 0xa0
{ "mov",   2,	0xa0, X, bwl_Suf|D|W,			{ Disp16|Disp32, Acc, 0 } },
{ "mov",   2,	0x88, X, bwl_Suf|D|W|Modrm,		{ Reg, Reg|AnyMem, 0 } },
{ "mov",   2,	0xb0, X, bwl_Suf|W|ShortForm,		{ Imm, Reg, 0 } },
{ "mov",   2,	0xc6, X, bwl_Suf|W|Modrm,		{ Imm, Reg|AnyMem, 0 } },
/* The next two instructions accept WordReg so that a segment register
   can be copied to a 32 bit register, and vice versa, without using a
   size prefix.  When moving to a 32 bit register, the upper 16 bits
   are set to an implementation defined value (on the Pentium Pro,
   the implementation defined value is zero).  */
{ "mov",   2,	0x8c, X, wl_Suf|Modrm,			{ SReg3|SReg2, WordReg|WordMem, 0 } },
{ "mov",   2,	0x8e, X, wl_Suf|Modrm|IgnoreSize,	{ WordReg|WordMem, SReg3|SReg2, 0 } },
/* move to/from control debug registers */
{ "mov",   2, 0x0f20, X, l_Suf|D|Modrm|IgnoreSize,	{ Control, Reg32, 0} },
{ "mov",   2, 0x0f21, X, l_Suf|D|Modrm|IgnoreSize,	{ Debug, Reg32, 0} },
{ "mov",   2, 0x0f24, X, l_Suf|D|Modrm|IgnoreSize,	{ Test, Reg32, 0} },

/* move with sign extend */
/* "movsbl" & "movsbw" must not be unified into "movsb" to avoid
   conflict with the "movs" string move instruction.  */
{"movsbl", 2, 0x0fbe, X, NoSuf|ReverseModrm,	{ Reg8|ByteMem, Reg32, 0} },
{"movsbw", 2, 0x0fbe, X, NoSuf|ReverseModrm,	{ Reg8|ByteMem, Reg16, 0} },
{"movswl", 2, 0x0fbf, X, NoSuf|ReverseModrm,	{ Reg16|ShortMem, Reg32, 0} },
/* Intel Syntax */
{"movsx",  2, 0x0fbf, X, w_Suf|ReverseModrm|IgnoreSize,    { Reg16|ShortMem, Reg32, 0} },
{"movsx",  2, 0x0fbe, X, b_Suf|ReverseModrm,    { Reg8|ByteMem, WordReg, 0} },

/* move with zero extend */
{"movzb",  2, 0x0fb6, X, wl_Suf|ReverseModrm,	{ Reg8|ByteMem, WordReg, 0} },
{"movzwl", 2, 0x0fb7, X, NoSuf|ReverseModrm,	{ Reg16|ShortMem, Reg32, 0} },
/* Intel Syntax */
{"movzx",  2, 0x0fb7, X, w_Suf|ReverseModrm|IgnoreSize,  { Reg16|ShortMem, Reg32, 0} },
{"movzx",  2, 0x0fb6, X, b_Suf|ReverseModrm,   { Reg8|ByteMem, WordReg, 0} },

/* push instructions */
{"push",   1,	0x50, X, wl_Suf|ShortForm,	{ WordReg,0,0 } },
{"push",   1,	0xff, 6, wl_Suf|Modrm,		{ WordReg|WordMem, 0, 0 } },
{"push",   1,	0x6a, X, wl_Suf,		{ Imm8S, 0, 0} },
{"push",   1,	0x68, X, wl_Suf,		{ Imm16|Imm32, 0, 0} },
{"push",   1,	0x06, X, wl_Suf|Seg2ShortForm,	{ SReg2,0,0 } },
{"push",   1, 0x0fa0, X, wl_Suf|Seg3ShortForm,	{ SReg3,0,0 } },
/* push all */
{"pusha",  0,	0x60, X, wl_Suf,		{ 0, 0, 0 } },

/* pop instructions */
{"pop",	   1,	0x58, X, wl_Suf|ShortForm,	{ WordReg,0,0 } },
{"pop",	   1,	0x8f, 0, wl_Suf|Modrm,		{ WordReg|WordMem, 0, 0 } },
#define POP_SEG_SHORT 0x07
{"pop",	   1,	0x07, X, wl_Suf|Seg2ShortForm,	{ SReg2,0,0 } },
{"pop",	   1, 0x0fa1, X, wl_Suf|Seg3ShortForm,	{ SReg3,0,0 } },
/* pop all */
{"popa",   0,	0x61, X, wl_Suf,		{ 0, 0, 0 } },

/* xchg exchange instructions
   xchg commutes:  we allow both operand orders */
{"xchg",   2,	0x90, X, wl_Suf|ShortForm,	{ WordReg, Acc, 0 } },
{"xchg",   2,	0x90, X, wl_Suf|ShortForm,	{ Acc, WordReg, 0 } },
{"xchg",   2,	0x86, X, bwl_Suf|W|Modrm,	{ Reg, Reg|AnyMem, 0 } },
{"xchg",   2,	0x86, X, bwl_Suf|W|Modrm,	{ Reg|AnyMem, Reg, 0 } },

/* in/out from ports */
{"in",	   2,	0xe4, X, bwl_Suf|W,		{ Imm8, Acc, 0 } },
{"in",	   2,	0xec, X, bwl_Suf|W,		{ InOutPortReg, Acc, 0 } },
{"in",	   1,	0xe4, X, bwl_Suf|W,		{ Imm8, 0, 0 } },
{"in",	   1,	0xec, X, bwl_Suf|W,		{ InOutPortReg, 0, 0 } },
{"out",	   2,	0xe6, X, bwl_Suf|W,		{ Acc, Imm8, 0 } },
{"out",	   2,	0xee, X, bwl_Suf|W,		{ Acc, InOutPortReg, 0 } },
{"out",	   1,	0xe6, X, bwl_Suf|W,		{ Imm8, 0, 0 } },
{"out",	   1,	0xee, X, bwl_Suf|W,		{ InOutPortReg, 0, 0 } },

/* load effective address */
{"lea",	   2, 0x8d,   X, wl_Suf|Modrm,		{ WordMem, WordReg, 0 } },

/* load segment registers from memory */
{"lds",	   2,	0xc5, X, wl_Suf|Modrm,		{ WordMem, WordReg, 0} },
{"les",	   2,	0xc4, X, wl_Suf|Modrm,		{ WordMem, WordReg, 0} },
{"lfs",	   2, 0x0fb4, X, wl_Suf|Modrm,		{ WordMem, WordReg, 0} },
{"lgs",	   2, 0x0fb5, X, wl_Suf|Modrm,		{ WordMem, WordReg, 0} },
{"lss",	   2, 0x0fb2, X, wl_Suf|Modrm,		{ WordMem, WordReg, 0} },

/* flags register instructions */
{"clc",	   0,	0xf8, X, NoSuf,			{ 0, 0, 0} },
{"cld",	   0,	0xfc, X, NoSuf,			{ 0, 0, 0} },
{"cli",	   0,	0xfa, X, NoSuf,			{ 0, 0, 0} },
{"clts",   0, 0x0f06, X, NoSuf,			{ 0, 0, 0} },
{"cmc",	   0,	0xf5, X, NoSuf,			{ 0, 0, 0} },
{"lahf",   0,	0x9f, X, NoSuf,			{ 0, 0, 0} },
{"sahf",   0,	0x9e, X, NoSuf,			{ 0, 0, 0} },
{"pushf",  0,	0x9c, X, wl_Suf,		{ 0, 0, 0} },
{"popf",   0,	0x9d, X, wl_Suf,		{ 0, 0, 0} },
{"stc",	   0,	0xf9, X, NoSuf,			{ 0, 0, 0} },
{"std",	   0,	0xfd, X, NoSuf,			{ 0, 0, 0} },
{"sti",	   0,	0xfb, X, NoSuf,			{ 0, 0, 0} },

/* arithmetic */
{"add",	   2,	0x00, X, bwl_Suf|D|W|Modrm,	{ Reg, Reg|AnyMem, 0} },
{"add",	   2,	0x83, 0, wl_Suf|Modrm,		{ Imm8S, WordReg|WordMem, 0} },
{"add",	   2,	0x04, X, bwl_Suf|W,		{ Imm, Acc, 0} },
{"add",	   2,	0x80, 0, bwl_Suf|W|Modrm,	{ Imm, Reg|AnyMem, 0} },

{"inc",	   1,	0x40, X, wl_Suf|ShortForm,	{ WordReg, 0, 0} },
{"inc",	   1,	0xfe, 0, bwl_Suf|W|Modrm,	{ Reg|AnyMem, 0, 0} },

{"sub",	   2,	0x28, X, bwl_Suf|D|W|Modrm,	{ Reg, Reg|AnyMem, 0} },
{"sub",	   2,	0x83, 5, wl_Suf|Modrm,		{ Imm8S, WordReg|WordMem, 0} },
{"sub",	   2,	0x2c, X, bwl_Suf|W,		{ Imm, Acc, 0} },
{"sub",	   2,	0x80, 5, bwl_Suf|W|Modrm,	{ Imm, Reg|AnyMem, 0} },

{"dec",	   1,	0x48, X, wl_Suf|ShortForm,	{ WordReg, 0, 0} },
{"dec",	   1,	0xfe, 1, bwl_Suf|W|Modrm,	{ Reg|AnyMem, 0, 0} },

{"sbb",	   2,	0x18, X, bwl_Suf|D|W|Modrm,	{ Reg, Reg|AnyMem, 0} },
{"sbb",	   2,	0x83, 3, wl_Suf|Modrm,		{ Imm8S, WordReg|WordMem, 0} },
{"sbb",	   2,	0x1c, X, bwl_Suf|W,		{ Imm, Acc, 0} },
{"sbb",	   2,	0x80, 3, bwl_Suf|W|Modrm,	{ Imm, Reg|AnyMem, 0} },

{"cmp",	   2,	0x38, X, bwl_Suf|D|W|Modrm,	{ Reg, Reg|AnyMem, 0} },
{"cmp",	   2,	0x83, 7, wl_Suf|Modrm,		{ Imm8S, WordReg|WordMem, 0} },
{"cmp",	   2,	0x3c, X, bwl_Suf|W,		{ Imm, Acc, 0} },
{"cmp",	   2,	0x80, 7, bwl_Suf|W|Modrm,	{ Imm, Reg|AnyMem, 0} },

{"test",   2,	0x84, X, bwl_Suf|W|Modrm,	{ Reg|AnyMem, Reg, 0} },
{"test",   2,	0x84, X, bwl_Suf|W|Modrm,	{ Reg, Reg|AnyMem, 0} },
{"test",   2,	0xa8, X, bwl_Suf|W,		{ Imm, Acc, 0} },
{"test",   2,	0xf6, 0, bwl_Suf|W|Modrm,	{ Imm, Reg|AnyMem, 0} },

{"and",	   2,	0x20, X, bwl_Suf|D|W|Modrm,	{ Reg, Reg|AnyMem, 0} },
{"and",	   2,	0x83, 4, wl_Suf|Modrm,		{ Imm8S, WordReg|WordMem, 0} },
{"and",	   2,	0x24, X, bwl_Suf|W,		{ Imm, Acc, 0} },
{"and",	   2,	0x80, 4, bwl_Suf|W|Modrm,	{ Imm, Reg|AnyMem, 0} },

{"or",	   2,	0x08, X, bwl_Suf|D|W|Modrm,	{ Reg, Reg|AnyMem, 0} },
{"or",	   2,	0x83, 1, wl_Suf|Modrm,		{ Imm8S, WordReg|WordMem, 0} },
{"or",	   2,	0x0c, X, bwl_Suf|W,		{ Imm, Acc, 0} },
{"or",	   2,	0x80, 1, bwl_Suf|W|Modrm,	{ Imm, Reg|AnyMem, 0} },

{"xor",	   2,	0x30, X, bwl_Suf|D|W|Modrm,	{ Reg, Reg|AnyMem, 0} },
{"xor",	   2,	0x83, 6, wl_Suf|Modrm,		{ Imm8S, WordReg|WordMem, 0} },
{"xor",	   2,	0x34, X, bwl_Suf|W,		{ Imm, Acc, 0} },
{"xor",	   2,	0x80, 6, bwl_Suf|W|Modrm,	{ Imm, Reg|AnyMem, 0} },

/* iclr with 1 operand is really xor with 2 operands. */
{"clr",	   1,	0x30, X, bwl_Suf|W|Modrm|regKludge,	{ Reg, 0, 0 } },

{"adc",	   2,	0x10, X, bwl_Suf|D|W|Modrm,	{ Reg, Reg|AnyMem, 0} },
{"adc",	   2,	0x83, 2, wl_Suf|Modrm,		{ Imm8S, WordReg|WordMem, 0} },
{"adc",	   2,	0x14, X, bwl_Suf|W,		{ Imm, Acc, 0} },
{"adc",	   2,	0x80, 2, bwl_Suf|W|Modrm,	{ Imm, Reg|AnyMem, 0} },

{"neg",	   1,	0xf6, 3, bwl_Suf|W|Modrm,	{ Reg|AnyMem, 0, 0} },
{"not",	   1,	0xf6, 2, bwl_Suf|W|Modrm,	{ Reg|AnyMem, 0, 0} },

{"aaa",	   0,	0x37, X, NoSuf,			{ 0, 0, 0} },
{"aas",	   0,	0x3f, X, NoSuf,			{ 0, 0, 0} },
{"daa",	   0,	0x27, X, NoSuf,			{ 0, 0, 0} },
{"das",	   0,	0x2f, X, NoSuf,			{ 0, 0, 0} },
{"aad",	   0, 0xd50a, X, NoSuf,			{ 0, 0, 0} },
{"aad",	   1,   0xd5, X, NoSuf,			{ Imm8S, 0, 0} },
{"aam",	   0, 0xd40a, X, NoSuf,			{ 0, 0, 0} },
{"aam",	   1,   0xd4, X, NoSuf,			{ Imm8S, 0, 0} },

/* conversion insns */
/* conversion:	intel naming */
{"cbw",	   0,	0x98, X, NoSuf|Size16,		{ 0, 0, 0} },
{"cwde",   0,	0x98, X, NoSuf|Size32,		{ 0, 0, 0} },
{"cwd",	   0,	0x99, X, NoSuf|Size16,		{ 0, 0, 0} },
{"cdq",	   0,	0x99, X, NoSuf|Size32,		{ 0, 0, 0} },
/*  att naming */
{"cbtw",   0,	0x98, X, NoSuf|Size16,		{ 0, 0, 0} },
{"cwtl",   0,	0x98, X, NoSuf|Size32,		{ 0, 0, 0} },
{"cwtd",   0,	0x99, X, NoSuf|Size16,		{ 0, 0, 0} },
{"cltd",   0,	0x99, X, NoSuf|Size32,		{ 0, 0, 0} },

/* Warning! the mul/imul (opcode 0xf6) must only have 1 operand!  They are
   expanding 64-bit multiplies, and *cannot* be selected to accomplish
   'imul %ebx, %eax' (opcode 0x0faf must be used in this case)
   These multiplies can only be selected with single operand forms.  */
{"mul",	   1,	0xf6, 4, bwl_Suf|W|Modrm,	{ Reg|AnyMem, 0, 0} },
{"imul",   1,	0xf6, 5, bwl_Suf|W|Modrm,	{ Reg|AnyMem, 0, 0} },
{"imul",   2, 0x0faf, X, wl_Suf|ReverseModrm,	{ WordReg|WordMem, WordReg, 0} },
{"imul",   3,	0x6b, X, wl_Suf|ReverseModrm,	{ Imm8S, WordReg|WordMem, WordReg} },
{"imul",   3,	0x69, X, wl_Suf|ReverseModrm,	{ Imm16|Imm32, WordReg|WordMem, WordReg} },
/* imul with 2 operands mimics imul with 3 by putting the register in
   both i.rm.reg & i.rm.regmem fields.  regKludge enables this
   transformation.  */
{"imul",   2,	0x6b, X, wl_Suf|Modrm|regKludge,{ Imm8S, WordReg, 0} },
{"imul",   2,	0x69, X, wl_Suf|Modrm|regKludge,{ Imm16|Imm32, WordReg, 0} },

{"div",	   1,	0xf6, 6, bwl_Suf|W|Modrm,	{ Reg|AnyMem, 0, 0} },
{"div",	   2,	0xf6, 6, bwl_Suf|W|Modrm,	{ Reg|AnyMem, Acc, 0} },
{"idiv",   1,	0xf6, 7, bwl_Suf|W|Modrm,	{ Reg|AnyMem, 0, 0} },
{"idiv",   2,	0xf6, 7, bwl_Suf|W|Modrm,	{ Reg|AnyMem, Acc, 0} },

{"rol",	   2,	0xd0, 0, bwl_Suf|W|Modrm,	{ Imm1, Reg|AnyMem, 0} },
{"rol",	   2,	0xc0, 0, bwl_Suf|W|Modrm,	{ Imm8, Reg|AnyMem, 0} },
{"rol",	   2,	0xd2, 0, bwl_Suf|W|Modrm,	{ ShiftCount, Reg|AnyMem, 0} },
{"rol",	   1,	0xd0, 0, bwl_Suf|W|Modrm,	{ Reg|AnyMem, 0, 0} },

{"ror",	   2,	0xd0, 1, bwl_Suf|W|Modrm,	{ Imm1, Reg|AnyMem, 0} },
{"ror",	   2,	0xc0, 1, bwl_Suf|W|Modrm,	{ Imm8, Reg|AnyMem, 0} },
{"ror",	   2,	0xd2, 1, bwl_Suf|W|Modrm,	{ ShiftCount, Reg|AnyMem, 0} },
{"ror",	   1,	0xd0, 1, bwl_Suf|W|Modrm,	{ Reg|AnyMem, 0, 0} },

{"rcl",	   2,	0xd0, 2, bwl_Suf|W|Modrm,	{ Imm1, Reg|AnyMem, 0} },
{"rcl",	   2,	0xc0, 2, bwl_Suf|W|Modrm,	{ Imm8, Reg|AnyMem, 0} },
{"rcl",	   2,	0xd2, 2, bwl_Suf|W|Modrm,	{ ShiftCount, Reg|AnyMem, 0} },
{"rcl",	   1,	0xd0, 2, bwl_Suf|W|Modrm,	{ Reg|AnyMem, 0, 0} },

{"rcr",	   2,	0xd0, 3, bwl_Suf|W|Modrm,	{ Imm1, Reg|AnyMem, 0} },
{"rcr",	   2,	0xc0, 3, bwl_Suf|W|Modrm,	{ Imm8, Reg|AnyMem, 0} },
{"rcr",	   2,	0xd2, 3, bwl_Suf|W|Modrm,	{ ShiftCount, Reg|AnyMem, 0} },
{"rcr",	   1,	0xd0, 3, bwl_Suf|W|Modrm,	{ Reg|AnyMem, 0, 0} },

{"sal",	   2,	0xd0, 4, bwl_Suf|W|Modrm,	{ Imm1, Reg|AnyMem, 0} },
{"sal",	   2,	0xc0, 4, bwl_Suf|W|Modrm,	{ Imm8, Reg|AnyMem, 0} },
{"sal",	   2,	0xd2, 4, bwl_Suf|W|Modrm,	{ ShiftCount, Reg|AnyMem, 0} },
{"sal",	   1,	0xd0, 4, bwl_Suf|W|Modrm,	{ Reg|AnyMem, 0, 0} },
{"shl",	   2,	0xd0, 4, bwl_Suf|W|Modrm,	{ Imm1, Reg|AnyMem, 0} },
{"shl",	   2,	0xc0, 4, bwl_Suf|W|Modrm,	{ Imm8, Reg|AnyMem, 0} },
{"shl",	   2,	0xd2, 4, bwl_Suf|W|Modrm,	{ ShiftCount, Reg|AnyMem, 0} },
{"shl",	   1,	0xd0, 4, bwl_Suf|W|Modrm,	{ Reg|AnyMem, 0, 0} },

{"shld",   3, 0x0fa4, X, wl_Suf|Modrm,		{ Imm8, WordReg, WordReg|WordMem} },
{"shld",   3, 0x0fa5, X, wl_Suf|Modrm,		{ ShiftCount, WordReg, WordReg|WordMem} },
{"shld",   2, 0x0fa5, X, wl_Suf|Modrm,		{ WordReg, WordReg|WordMem, 0} },

{"shr",	   2,	0xd0, 5, bwl_Suf|W|Modrm,	{ Imm1, Reg|AnyMem, 0} },
{"shr",	   2,	0xc0, 5, bwl_Suf|W|Modrm,	{ Imm8, Reg|AnyMem, 0} },
{"shr",	   2,	0xd2, 5, bwl_Suf|W|Modrm,	{ ShiftCount, Reg|AnyMem, 0} },
{"shr",	   1,	0xd0, 5, bwl_Suf|W|Modrm,	{ Reg|AnyMem, 0, 0} },

{"shrd",   3, 0x0fac, X, wl_Suf|Modrm,		{ Imm8, WordReg, WordReg|WordMem} },
{"shrd",   3, 0x0fad, X, wl_Suf|Modrm,		{ ShiftCount, WordReg, WordReg|WordMem} },
{"shrd",   2, 0x0fad, X, wl_Suf|Modrm,		{ WordReg, WordReg|WordMem, 0} },

{"sar",	   2,	0xd0, 7, bwl_Suf|W|Modrm,	{ Imm1, Reg|AnyMem, 0} },
{"sar",	   2,	0xc0, 7, bwl_Suf|W|Modrm,	{ Imm8, Reg|AnyMem, 0} },
{"sar",	   2,	0xd2, 7, bwl_Suf|W|Modrm,	{ ShiftCount, Reg|AnyMem, 0} },
{"sar",	   1,	0xd0, 7, bwl_Suf|W|Modrm,	{ Reg|AnyMem, 0, 0} },

/* control transfer instructions */
{"call",   1,	0xe8, X, wl_Suf|JumpDword,	{ Disp16|Disp32, 0, 0} },
{"call",   1,	0xff, 2, wl_Suf|Modrm,		{ WordReg|WordMem|JumpAbsolute, 0, 0} },
/* Intel Syntax */
{"call",   2, 0x9a, X, wl_Suf|JumpInterSegment, { Imm16, Imm16|Imm32, 0} },
{"lcall",  2,	0x9a, X, wl_Suf|JumpInterSegment, { Imm16, Imm16|Imm32, 0} },
{"lcall",  1,	0xff, 3, wl_Suf|Modrm,		{ WordMem, 0, 0} },

#define JUMP_PC_RELATIVE 0xeb
{"jmp",	   1,	0xeb, X, NoSuf|Jump,		{ Disp, 0, 0} },
{"jmp",	   1,	0xff, 4, wl_Suf|Modrm,		{ WordReg|WordMem|JumpAbsolute, 0, 0} },
/* Intel Syntax */
{"jmp",   2,	0xea, X, wl_Suf|JumpInterSegment, { Imm16, Imm16|Imm32, 0} },
{"jmp",   1,	0xff, 5, wl_Suf|Modrm,		{ WordMem, 0, 0} },
{"ljmp",   2,	0xea, X, wl_Suf|JumpInterSegment, { Imm16, Imm16|Imm32, 0} },
{"ljmp",   1,	0xff, 5, wl_Suf|Modrm,		{ WordMem, 0, 0} },

{"ret",	   0,	0xc3, X, wl_Suf,		{ 0, 0, 0} },
{"ret",	   1,	0xc2, X, wl_Suf,		{ Imm16, 0, 0} },
{"lret",   0,	0xcb, X, wl_Suf,		{ 0, 0, 0} },
{"lret",   1,	0xca, X, wl_Suf,		{ Imm16, 0, 0} },
{"enter",  2,	0xc8, X, wl_Suf,		{ Imm16, Imm8, 0} },
{"leave",  0,	0xc9, X, wl_Suf,		{ 0, 0, 0} },

/* conditional jumps */
{"jo",	   1,	0x70, X, NoSuf|Jump,		{ Disp, 0, 0} },
{"jno",	   1,	0x71, X, NoSuf|Jump,		{ Disp, 0, 0} },
{"jb",	   1,	0x72, X, NoSuf|Jump,		{ Disp, 0, 0} },
{"jc",	   1,	0x72, X, NoSuf|Jump,		{ Disp, 0, 0} },
{"jnae",   1,	0x72, X, NoSuf|Jump,		{ Disp, 0, 0} },
{"jnb",	   1,	0x73, X, NoSuf|Jump,		{ Disp, 0, 0} },
{"jnc",	   1,	0x73, X, NoSuf|Jump,		{ Disp, 0, 0} },
{"jae",	   1,	0x73, X, NoSuf|Jump,		{ Disp, 0, 0} },
{"je",	   1,	0x74, X, NoSuf|Jump,		{ Disp, 0, 0} },
{"jz",	   1,	0x74, X, NoSuf|Jump,		{ Disp, 0, 0} },
{"jne",	   1,	0x75, X, NoSuf|Jump,		{ Disp, 0, 0} },
{"jnz",	   1,	0x75, X, NoSuf|Jump,		{ Disp, 0, 0} },
{"jbe",	   1,	0x76, X, NoSuf|Jump,		{ Disp, 0, 0} },
{"jna",	   1,	0x76, X, NoSuf|Jump,		{ Disp, 0, 0} },
{"jnbe",   1,	0x77, X, NoSuf|Jump,		{ Disp, 0, 0} },
{"ja",	   1,	0x77, X, NoSuf|Jump,		{ Disp, 0, 0} },
{"js",	   1,	0x78, X, NoSuf|Jump,		{ Disp, 0, 0} },
{"jns",	   1,	0x79, X, NoSuf|Jump,		{ Disp, 0, 0} },
{"jp",	   1,	0x7a, X, NoSuf|Jump,		{ Disp, 0, 0} },
{"jpe",	   1,	0x7a, X, NoSuf|Jump,		{ Disp, 0, 0} },
{"jnp",	   1,	0x7b, X, NoSuf|Jump,		{ Disp, 0, 0} },
{"jpo",	   1,	0x7b, X, NoSuf|Jump,		{ Disp, 0, 0} },
{"jl",	   1,	0x7c, X, NoSuf|Jump,		{ Disp, 0, 0} },
{"jnge",   1,	0x7c, X, NoSuf|Jump,		{ Disp, 0, 0} },
{"jnl",	   1,	0x7d, X, NoSuf|Jump,		{ Disp, 0, 0} },
{"jge",	   1,	0x7d, X, NoSuf|Jump,		{ Disp, 0, 0} },
{"jle",	   1,	0x7e, X, NoSuf|Jump,		{ Disp, 0, 0} },
{"jng",	   1,	0x7e, X, NoSuf|Jump,		{ Disp, 0, 0} },
{"jnle",   1,	0x7f, X, NoSuf|Jump,		{ Disp, 0, 0} },
{"jg",	   1,	0x7f, X, NoSuf|Jump,		{ Disp, 0, 0} },

/* jcxz vs. jecxz is chosen on the basis of the address size prefix.  */
{"jcxz",   1,	0xe3, X, NoSuf|JumpByte|Size16, { Disp, 0, 0} },
{"jecxz",  1,	0xe3, X, NoSuf|JumpByte|Size32, { Disp, 0, 0} },

/* The loop instructions also use the address size prefix to select
   %cx rather than %ecx for the loop count, so the `w' form of these
   instructions emit an address size prefix rather than a data size
   prefix.  */
{"loop",   1,	0xe2, X, wl_Suf|JumpByte,	{ Disp, 0, 0} },
{"loopz",  1,	0xe1, X, wl_Suf|JumpByte,	{ Disp, 0, 0} },
{"loope",  1,	0xe1, X, wl_Suf|JumpByte,	{ Disp, 0, 0} },
{"loopnz", 1,	0xe0, X, wl_Suf|JumpByte,	{ Disp, 0, 0} },
{"loopne", 1,	0xe0, X, wl_Suf|JumpByte,	{ Disp, 0, 0} },

/* set byte on flag instructions */
{"seto",   1, 0x0f90, 0, b_Suf|Modrm,		{ Reg8|ByteMem, 0, 0} },
{"setno",  1, 0x0f91, 0, b_Suf|Modrm,		{ Reg8|ByteMem, 0, 0} },
{"setb",   1, 0x0f92, 0, b_Suf|Modrm,		{ Reg8|ByteMem, 0, 0} },
{"setc",   1, 0x0f92, 0, b_Suf|Modrm,		{ Reg8|ByteMem, 0, 0} },
{"setnae", 1, 0x0f92, 0, b_Suf|Modrm,		{ Reg8|ByteMem, 0, 0} },
{"setnb",  1, 0x0f93, 0, b_Suf|Modrm,		{ Reg8|ByteMem, 0, 0} },
{"setnc",  1, 0x0f93, 0, b_Suf|Modrm,		{ Reg8|ByteMem, 0, 0} },
{"setae",  1, 0x0f93, 0, b_Suf|Modrm,		{ Reg8|ByteMem, 0, 0} },
{"sete",   1, 0x0f94, 0, b_Suf|Modrm,		{ Reg8|ByteMem, 0, 0} },
{"setz",   1, 0x0f94, 0, b_Suf|Modrm,		{ Reg8|ByteMem, 0, 0} },
{"setne",  1, 0x0f95, 0, b_Suf|Modrm,		{ Reg8|ByteMem, 0, 0} },
{"setnz",  1, 0x0f95, 0, b_Suf|Modrm,		{ Reg8|ByteMem, 0, 0} },
{"setbe",  1, 0x0f96, 0, b_Suf|Modrm,		{ Reg8|ByteMem, 0, 0} },
{"setna",  1, 0x0f96, 0, b_Suf|Modrm,		{ Reg8|ByteMem, 0, 0} },
{"setnbe", 1, 0x0f97, 0, b_Suf|Modrm,		{ Reg8|ByteMem, 0, 0} },
{"seta",   1, 0x0f97, 0, b_Suf|Modrm,		{ Reg8|ByteMem, 0, 0} },
{"sets",   1, 0x0f98, 0, b_Suf|Modrm,		{ Reg8|ByteMem, 0, 0} },
{"setns",  1, 0x0f99, 0, b_Suf|Modrm,		{ Reg8|ByteMem, 0, 0} },
{"setp",   1, 0x0f9a, 0, b_Suf|Modrm,		{ Reg8|ByteMem, 0, 0} },
{"setpe",  1, 0x0f9a, 0, b_Suf|Modrm,		{ Reg8|ByteMem, 0, 0} },
{"setnp",  1, 0x0f9b, 0, b_Suf|Modrm,		{ Reg8|ByteMem, 0, 0} },
{"setpo",  1, 0x0f9b, 0, b_Suf|Modrm,		{ Reg8|ByteMem, 0, 0} },
{"setl",   1, 0x0f9c, 0, b_Suf|Modrm,		{ Reg8|ByteMem, 0, 0} },
{"setnge", 1, 0x0f9c, 0, b_Suf|Modrm,		{ Reg8|ByteMem, 0, 0} },
{"setnl",  1, 0x0f9d, 0, b_Suf|Modrm,		{ Reg8|ByteMem, 0, 0} },
{"setge",  1, 0x0f9d, 0, b_Suf|Modrm,		{ Reg8|ByteMem, 0, 0} },
{"setle",  1, 0x0f9e, 0, b_Suf|Modrm,		{ Reg8|ByteMem, 0, 0} },
{"setng",  1, 0x0f9e, 0, b_Suf|Modrm,		{ Reg8|ByteMem, 0, 0} },
{"setnle", 1, 0x0f9f, 0, b_Suf|Modrm,		{ Reg8|ByteMem, 0, 0} },
{"setg",   1, 0x0f9f, 0, b_Suf|Modrm,		{ Reg8|ByteMem, 0, 0} },

/* string manipulation */
{"cmps",   0,	0xa6, X, bwld_Suf|W|IsString,	{ 0, 0, 0} },
{"cmps",   2,	0xa6, X, bwld_Suf|W|IsString,	{ AnyMem|EsSeg, AnyMem, 0} },
{"scmp",   0,	0xa6, X, bwld_Suf|W|IsString,	{ 0, 0, 0} },
{"scmp",   2,	0xa6, X, bwld_Suf|W|IsString,	{ AnyMem|EsSeg, AnyMem, 0} },
{"ins",	   0,	0x6c, X, bwld_Suf|W|IsString,	{ 0, 0, 0} },
{"ins",	   2,	0x6c, X, bwld_Suf|W|IsString,	{ InOutPortReg, AnyMem|EsSeg, 0} },
{"outs",   0,	0x6e, X, bwld_Suf|W|IsString,	{ 0, 0, 0} },
{"outs",   2,	0x6e, X, bwld_Suf|W|IsString,	{ AnyMem, InOutPortReg, 0} },
{"lods",   0,	0xac, X, bwld_Suf|W|IsString,	{ 0, 0, 0} },
{"lods",   1,	0xac, X, bwld_Suf|W|IsString,	{ AnyMem, 0, 0} },
{"lods",   2,	0xac, X, bwld_Suf|W|IsString,	{ AnyMem, Acc, 0} },
{"slod",   0,	0xac, X, bwld_Suf|W|IsString,	{ 0, 0, 0} },
{"slod",   1,	0xac, X, bwld_Suf|W|IsString,	{ AnyMem, 0, 0} },
{"slod",   2,	0xac, X, bwld_Suf|W|IsString,	{ AnyMem, Acc, 0} },
{"movs",   0,	0xa4, X, bwld_Suf|W|IsString,	{ 0, 0, 0} },
{"movs",   2,	0xa4, X, bwld_Suf|W|IsString,	{ AnyMem, AnyMem|EsSeg, 0} },
{"smov",   0,	0xa4, X, bwld_Suf|W|IsString,	{ 0, 0, 0} },
{"smov",   2,	0xa4, X, bwld_Suf|W|IsString,	{ AnyMem, AnyMem|EsSeg, 0} },
{"scas",   0,	0xae, X, bwld_Suf|W|IsString,	{ 0, 0, 0} },
{"scas",   1,	0xae, X, bwld_Suf|W|IsString,	{ AnyMem|EsSeg, 0, 0} },
{"scas",   2,	0xae, X, bwld_Suf|W|IsString,	{ AnyMem|EsSeg, Acc, 0} },
{"ssca",   0,	0xae, X, bwld_Suf|W|IsString,	{ 0, 0, 0} },
{"ssca",   1,	0xae, X, bwld_Suf|W|IsString,	{ AnyMem|EsSeg, 0, 0} },
{"ssca",   2,	0xae, X, bwld_Suf|W|IsString,	{ AnyMem|EsSeg, Acc, 0} },
{"stos",   0,	0xaa, X, bwld_Suf|W|IsString,	{ 0, 0, 0} },
{"stos",   1,	0xaa, X, bwld_Suf|W|IsString,	{ AnyMem|EsSeg, 0, 0} },
{"stos",   2,	0xaa, X, bwld_Suf|W|IsString,	{ Acc, AnyMem|EsSeg, 0} },
{"ssto",   0,	0xaa, X, bwld_Suf|W|IsString,	{ 0, 0, 0} },
{"ssto",   1,	0xaa, X, bwld_Suf|W|IsString,	{ AnyMem|EsSeg, 0, 0} },
{"ssto",   2,	0xaa, X, bwld_Suf|W|IsString,	{ Acc, AnyMem|EsSeg, 0} },
{"xlat",   0,	0xd7, X, b_Suf|IsString,	{ 0, 0, 0} },
{"xlat",   1,	0xd7, X, b_Suf|IsString,	{ AnyMem, 0, 0} },

/* bit manipulation */
{"bsf",	   2, 0x0fbc, X, wl_Suf|ReverseModrm,	{ WordReg|WordMem, WordReg, 0} },
{"bsr",	   2, 0x0fbd, X, wl_Suf|ReverseModrm,	{ WordReg|WordMem, WordReg, 0} },
{"bt",	   2, 0x0fa3, X, wl_Suf|Modrm,		{ WordReg, WordReg|WordMem, 0} },
{"bt",	   2, 0x0fba, 4, wl_Suf|Modrm,		{ Imm8, WordReg|WordMem, 0} },
{"btc",	   2, 0x0fbb, X, wl_Suf|Modrm,		{ WordReg, WordReg|WordMem, 0} },
{"btc",	   2, 0x0fba, 7, wl_Suf|Modrm,		{ Imm8, WordReg|WordMem, 0} },
{"btr",	   2, 0x0fb3, X, wl_Suf|Modrm,		{ WordReg, WordReg|WordMem, 0} },
{"btr",	   2, 0x0fba, 6, wl_Suf|Modrm,		{ Imm8, WordReg|WordMem, 0} },
{"bts",	   2, 0x0fab, X, wl_Suf|Modrm,		{ WordReg, WordReg|WordMem, 0} },
{"bts",	   2, 0x0fba, 5, wl_Suf|Modrm,		{ Imm8, WordReg|WordMem, 0} },

/* interrupts & op. sys insns */
/* See gas/config/tc-i386.c for conversion of 'int $3' into the special
   int 3 insn. */
#define INT_OPCODE 0xcd
#define INT3_OPCODE 0xcc
{"int",	   1,	0xcd, X, NoSuf,			{ Imm8, 0, 0} },
{"int3",   0,	0xcc, X, NoSuf,			{ 0, 0, 0} },
{"into",   0,	0xce, X, NoSuf,			{ 0, 0, 0} },
{"iret",   0,	0xcf, X, wl_Suf,		{ 0, 0, 0} },
/* i386sl, i486sl, later 486, and Pentium */
{"rsm",	   0, 0x0faa, X, NoSuf,			{ 0, 0, 0} },

{"bound",  2,	0x62, X, wl_Suf|Modrm,		{ WordReg, WordMem, 0} },

{"hlt",	   0,	0xf4, X, NoSuf,			{ 0, 0, 0} },
/* nop is actually 'xchgl %eax, %eax' */
{"nop",	   0,	0x90, X, NoSuf,			{ 0, 0, 0} },

/* protection control */
{"arpl",   2,	0x63, X, NoSuf|Modrm|IgnoreSize,{ Reg16, Reg16|ShortMem, 0} },
{"lar",	   2, 0x0f02, X, wl_Suf|ReverseModrm,	{ WordReg|WordMem, WordReg, 0} },
{"lgdt",   1, 0x0f01, 2, wl_Suf|Modrm,		{ WordMem, 0, 0} },
{"lidt",   1, 0x0f01, 3, wl_Suf|Modrm,		{ WordMem, 0, 0} },
{"lldt",   1, 0x0f00, 2, NoSuf|Modrm|IgnoreSize,{ Reg16|ShortMem, 0, 0} },
{"lmsw",   1, 0x0f01, 6, NoSuf|Modrm|IgnoreSize,{ Reg16|ShortMem, 0, 0} },
{"lsl",	   2, 0x0f03, X, wl_Suf|ReverseModrm,	{ WordReg|WordMem, WordReg, 0} },
{"ltr",	   1, 0x0f00, 3, NoSuf|Modrm|IgnoreSize,{ Reg16|ShortMem, 0, 0} },

{"sgdt",   1, 0x0f01, 0, wl_Suf|Modrm,		{ WordMem, 0, 0} },
{"sidt",   1, 0x0f01, 1, wl_Suf|Modrm,		{ WordMem, 0, 0} },
{"sldt",   1, 0x0f00, 0, wl_Suf|Modrm,		{ WordReg|WordMem, 0, 0} },
{"smsw",   1, 0x0f01, 4, wl_Suf|Modrm,		{ WordReg|WordMem, 0, 0} },
{"str",	   1, 0x0f00, 1, NoSuf|Modrm|IgnoreSize,{ Reg16|ShortMem, 0, 0} },

{"verr",   1, 0x0f00, 4, NoSuf|Modrm|IgnoreSize,{ Reg16|ShortMem, 0, 0} },
{"verw",   1, 0x0f00, 5, NoSuf|Modrm|IgnoreSize,{ Reg16|ShortMem, 0, 0} },

/* floating point instructions */

/* load */
{"fld",	   1, 0xd9c0, X, FP|ShortForm,		{ FloatReg, 0, 0} }, /* register */
{"fld",	   1,	0xd9, 0, sld_FP|FloatMF|Modrm,	{ LongMem|LLongMem, 0, 0} }, /* %st0 <-- mem float/double */
{"fld",	   1, 0xd9c0, X, l_FP|ShortForm|Ugh,	{ FloatReg, 0, 0} },
/* Intel Syntax */
{"fld",    1,	0xdb, 5, x_FP|Modrm,		{ LLongMem, 0, 0} }, /* %st0 <-- mem efloat */
{"fild",   1,	0xdf, 0, sl_Suf|FloatMF|Modrm,	{ ShortMem|LongMem, 0, 0} }, /* %st0 <-- mem word(16)/dword(32) */
/* Intel Syntax */
{"fild",  1,	0xdf, 5, d_Suf|IgnoreSize|Modrm,{ LLongMem, 0, 0} }, /* %st0 <-- mem qword (64) */
{"fildq",  1,	0xdf, 5, FP|Modrm,		{ LLongMem, 0, 0} }, /* %st0 <-- mem qword (64) */
{"fildll", 1,	0xdf, 5, FP|Modrm,		{ LLongMem, 0, 0} }, /* %st0 <-- mem qword (64) */
{"fldt",   1,	0xdb, 5, FP|Modrm,		{ LLongMem, 0, 0} }, /* %st0 <-- mem efloat */
{"fbld",   1,	0xdf, 4, FP|Modrm,		{ LLongMem, 0, 0} }, /* %st0 <-- mem bcd */

/* store (no pop) */
{"fst",	   1, 0xddd0, X, FP|ShortForm,		{ FloatReg, 0, 0} }, /* register */
{"fst",	   1,	0xd9, 2, sld_FP|FloatMF|Modrm,	{ LongMem|LLongMem, 0, 0} }, /* %st0 --> mem float/double */
{"fst",	   1, 0xddd0, X, l_FP|ShortForm|Ugh,	{ FloatReg, 0, 0} },
{"fist",   1,	0xdf, 2, sld_FP|FloatMF|Modrm,	{ ShortMem|LongMem, 0, 0} }, /* %st0 --> mem word(16)/dword(32) */

/* store (with pop) */
{"fstp",   1, 0xddd8, X, FP|ShortForm,		{ FloatReg, 0, 0} }, /* register */
{"fstp",   1,	0xd9, 3, sld_FP|FloatMF|Modrm,	{ LongMem|LLongMem, 0, 0} }, /* %st0 --> mem float/double */
{"fstp",   1, 0xddd8, X, l_FP|ShortForm|Ugh,	{ FloatReg, 0, 0} },
/* Intel Syntax */
{"fstp",   1,	0xdb, 7, x_FP|Modrm,		{ LLongMem, 0, 0} }, /* %st0 --> mem efloat */
{"fistp",  1,	0xdf, 3, sl_FP|FloatMF|Modrm,	{ ShortMem|LongMem, 0, 0} }, /* %st0 --> mem word(16)/dword(32) */
/* Intel Syntax */
{"fistp", 1,	0xdf, 7, d_FP|Modrm,		{ LLongMem, 0, 0} }, /* %st0 --> mem qword (64) */
{"fistpq", 1,	0xdf, 7, FP|Modrm,		{ LLongMem, 0, 0} }, /* %st0 --> mem qword (64) */
{"fistpll",1,	0xdf, 7, FP|Modrm,		{ LLongMem, 0, 0} }, /* %st0 --> mem qword (64) */
{"fstpt",  1,	0xdb, 7, FP|Modrm,		{ LLongMem, 0, 0} }, /* %st0 --> mem efloat */
{"fbstp",  1,	0xdf, 6, FP|Modrm,		{ LLongMem, 0, 0} }, /* %st0 --> mem bcd */

/* exchange %st<n> with %st0 */
{"fxch",   1, 0xd9c8, X, FP|ShortForm,		{ FloatReg, 0, 0} },
{"fxch",   0, 0xd9c9, X, FP,			{ 0, 0, 0} },	     /* alias for fxch %st(1) */

/* comparison (without pop) */
{"fcom",   1, 0xd8d0, X, FP|ShortForm,		{ FloatReg, 0, 0} },
{"fcom",   0, 0xd8d1, X, FP,			{ 0, 0, 0} },	     /* alias for fcom %st(1) */
{"fcom",   1,	0xd8, 2, sld_FP|FloatMF|Modrm,	{ LongMem|LLongMem, 0, 0} }, /* compare %st0, mem float/double */
{"fcom",   1, 0xd8d0, X, l_FP|ShortForm|Ugh,	{ FloatReg, 0, 0} },
{"ficom",  1,	0xde, 2, sl_FP|FloatMF|Modrm,	{ ShortMem|LongMem, 0, 0} }, /* compare %st0, mem word/dword */

/* comparison (with pop) */
{"fcomp",  1, 0xd8d8, X, FP|ShortForm,		{ FloatReg, 0, 0} },
{"fcomp",  0, 0xd8d9, X, FP,			{ 0, 0, 0} },	     /* alias for fcomp %st(1) */
{"fcomp",  1,	0xd8, 3, sld_FP|FloatMF|Modrm,	{ LongMem|LLongMem, 0, 0} }, /* compare %st0, mem float/double */
{"fcomp",  1, 0xd8d8, X, l_FP|ShortForm|Ugh,	{ FloatReg, 0, 0} },
{"ficomp", 1,	0xde, 3, sl_FP|FloatMF|Modrm,	{ ShortMem|LongMem, 0, 0} }, /* compare %st0, mem word/dword */
{"fcompp", 0, 0xded9, X, FP,			{ 0, 0, 0} },	     /* compare %st0, %st1 & pop 2 */

/* unordered comparison (with pop) */
{"fucom",  1, 0xdde0, X, FP|ShortForm,		{ FloatReg, 0, 0} },
{"fucom",  0, 0xdde1, X, FP,			{ 0, 0, 0} },	     /* alias for fucom %st(1) */
{"fucomp", 1, 0xdde8, X, FP|ShortForm,		{ FloatReg, 0, 0} },
{"fucomp", 0, 0xdde9, X, FP,			{ 0, 0, 0} },	     /* alias for fucomp %st(1) */
{"fucompp",0, 0xdae9, X, FP,			{ 0, 0, 0} },	     /* ucompare %st0, %st1 & pop twice */

{"ftst",   0, 0xd9e4, X, FP,			{ 0, 0, 0} },	     /* test %st0 */
{"fxam",   0, 0xd9e5, X, FP,			{ 0, 0, 0} },	     /* examine %st0 */

/* load constants into %st0 */
{"fld1",   0, 0xd9e8, X, FP,			{ 0, 0, 0} },	     /* %st0 <-- 1.0 */
{"fldl2t", 0, 0xd9e9, X, FP,			{ 0, 0, 0} },	     /* %st0 <-- log2(10) */
{"fldl2e", 0, 0xd9ea, X, FP,			{ 0, 0, 0} },	     /* %st0 <-- log2(e) */
{"fldpi",  0, 0xd9eb, X, FP,			{ 0, 0, 0} },	     /* %st0 <-- pi */
{"fldlg2", 0, 0xd9ec, X, FP,			{ 0, 0, 0} },	     /* %st0 <-- log10(2) */
{"fldln2", 0, 0xd9ed, X, FP,			{ 0, 0, 0} },	     /* %st0 <-- ln(2) */
{"fldz",   0, 0xd9ee, X, FP,			{ 0, 0, 0} },	     /* %st0 <-- 0.0 */

/* arithmetic */

/* add */
{"fadd",   2, 0xd8c0, X, FP|ShortForm|FloatD,	{ FloatReg, FloatAcc, 0} },
{"fadd",   1, 0xd8c0, X, FP|ShortForm,		{ FloatReg, 0, 0} }, /* alias for fadd %st(i), %st */
#if UNIXWARE_COMPAT
{"fadd",   0, 0xdec1, X, FP|Ugh,		{ 0, 0, 0} },	     /* alias for faddp */
#endif
{"fadd",   1,	0xd8, 0, sld_FP|FloatMF|Modrm,	{ LongMem|LLongMem, 0, 0} },
{"fiadd",  1,	0xde, 0, sld_FP|FloatMF|Modrm,	{ ShortMem|LongMem, 0, 0} },

{"faddp",  2, 0xdec0, X, FP|ShortForm,		{ FloatAcc, FloatReg, 0} },
{"faddp",  1, 0xdec0, X, FP|ShortForm,		{ FloatReg, 0, 0} },
{"faddp",  0, 0xdec1, X, FP,			{ 0, 0, 0} },	     /* alias for faddp %st, %st(1) */
{"faddp",  2, 0xdec0, X, FP|ShortForm|Ugh,	{ FloatReg, FloatAcc, 0} },

/* subtract */
{"fsub",   2, 0xd8e0, X, FP|ShortForm|FloatDR,	{ FloatReg, FloatAcc, 0} },
{"fsub",   1, 0xd8e0, X, FP|ShortForm,		{ FloatReg, 0, 0} },
#if UNIXWARE_COMPAT
{"fsub",   0, 0xdee1, X, FP|Ugh,		{ 0, 0, 0} },	     /* alias for fsubp */
#endif
{"fsub",   1,	0xd8, 4, sld_FP|FloatMF|Modrm,	{ LongMem|LLongMem, 0, 0} },
{"fisub",  1,	0xde, 4, sl_FP|FloatMF|Modrm,	{ ShortMem|LongMem, 0, 0} },

#if UNIXWARE_COMPAT
{"fsubp",  2, 0xdee0, X, FP|ShortForm,		{ FloatAcc, FloatReg, 0} },
{"fsubp",  1, 0xdee0, X, FP|ShortForm,		{ FloatReg, 0, 0} },
{"fsubp",  0, 0xdee1, X, FP,			{ 0, 0, 0} },
{"fsubp",  2, 0xdee0, X, FP|ShortForm|Ugh,	{ FloatReg, FloatAcc, 0} },
#else
{"fsubp",  2, 0xdee8, X, FP|ShortForm,		{ FloatAcc, FloatReg, 0} },
{"fsubp",  1, 0xdee8, X, FP|ShortForm,		{ FloatReg, 0, 0} },
{"fsubp",  0, 0xdee9, X, FP,			{ 0, 0, 0} },
#endif

/* subtract reverse */
{"fsubr",  2, 0xd8e8, X, FP|ShortForm|FloatDR,	{ FloatReg, FloatAcc, 0} },
{"fsubr",  1, 0xd8e8, X, FP|ShortForm,		{ FloatReg, 0, 0} },
#if UNIXWARE_COMPAT
{"fsubr",  0, 0xdee9, X, FP|Ugh,		{ 0, 0, 0} },	     /* alias for fsubrp */
#endif
{"fsubr",  1,	0xd8, 5, sld_FP|FloatMF|Modrm,	{ LongMem|LLongMem, 0, 0} },
{"fisubr", 1,	0xde, 5, sl_FP|FloatMF|Modrm,	{ ShortMem|LongMem, 0, 0} },

#if UNIXWARE_COMPAT
{"fsubrp", 2, 0xdee8, X, FP|ShortForm,		{ FloatAcc, FloatReg, 0} },
{"fsubrp", 1, 0xdee8, X, FP|ShortForm,		{ FloatReg, 0, 0} },
{"fsubrp", 0, 0xdee9, X, FP,			{ 0, 0, 0} },
{"fsubrp", 2, 0xdee8, X, FP|ShortForm|Ugh,	{ FloatReg, FloatAcc, 0} },
#else
{"fsubrp", 2, 0xdee0, X, FP|ShortForm,		{ FloatAcc, FloatReg, 0} },
{"fsubrp", 1, 0xdee0, X, FP|ShortForm,		{ FloatReg, 0, 0} },
{"fsubrp", 0, 0xdee1, X, FP,			{ 0, 0, 0} },
#endif

/* multiply */
{"fmul",   2, 0xd8c8, X, FP|ShortForm|FloatD,	{ FloatReg, FloatAcc, 0} },
{"fmul",   1, 0xd8c8, X, FP|ShortForm,		{ FloatReg, 0, 0} },
#if UNIXWARE_COMPAT
{"fmul",   0, 0xdec9, X, FP|Ugh,		{ 0, 0, 0} },	     /* alias for fmulp */
#endif
{"fmul",   1,	0xd8, 1, sld_FP|FloatMF|Modrm,	{ LongMem|LLongMem, 0, 0} },
{"fimul",  1,	0xde, 1, sld_FP|FloatMF|Modrm,	{ ShortMem|LongMem, 0, 0} },

{"fmulp",  2, 0xdec8, X, FP|ShortForm,		{ FloatAcc, FloatReg, 0} },
{"fmulp",  1, 0xdec8, X, FP|ShortForm,		{ FloatReg, 0, 0} },
{"fmulp",  0, 0xdec9, X, FP,			{ 0, 0, 0} },
{"fmulp",  2, 0xdec8, X, FP|ShortForm|Ugh,	{ FloatReg, FloatAcc, 0} },

/* divide */
{"fdiv",   2, 0xd8f0, X, FP|ShortForm|FloatDR,	{ FloatReg, FloatAcc, 0} },
{"fdiv",   1, 0xd8f0, X, FP|ShortForm,		{ FloatReg, 0, 0} },
#if UNIXWARE_COMPAT
{"fdiv",   0, 0xdef1, X, FP|Ugh,		{ 0, 0, 0} },	     /* alias for fdivp */
#endif
{"fdiv",   1,	0xd8, 6, sld_FP|FloatMF|Modrm,	{ LongMem|LLongMem, 0, 0} },
{"fidiv",  1,	0xde, 6, sld_FP|FloatMF|Modrm,	{ ShortMem|LongMem, 0, 0} },

#if UNIXWARE_COMPAT
{"fdivp",  2, 0xdef0, X, FP|ShortForm,		{ FloatAcc, FloatReg, 0} },
{"fdivp",  1, 0xdef0, X, FP|ShortForm,		{ FloatReg, 0, 0} },
{"fdivp",  0, 0xdef1, X, FP,			{ 0, 0, 0} },
{"fdivp",  2, 0xdef0, X, FP|ShortForm|Ugh,	{ FloatReg, FloatAcc, 0} },
#else
{"fdivp",  2, 0xdef8, X, FP|ShortForm,		{ FloatAcc, FloatReg, 0} },
{"fdivp",  1, 0xdef8, X, FP|ShortForm,		{ FloatReg, 0, 0} },
{"fdivp",  0, 0xdef9, X, FP,			{ 0, 0, 0} },
#endif

/* divide reverse */
{"fdivr",  2, 0xd8f8, X, FP|ShortForm|FloatDR,	{ FloatReg, FloatAcc, 0} },
{"fdivr",  1, 0xd8f8, X, FP|ShortForm,		{ FloatReg, 0, 0} },
#if UNIXWARE_COMPAT
{"fdivr",  0, 0xdef9, X, FP|Ugh,		{ 0, 0, 0} },	     /* alias for fdivrp */
#endif
{"fdivr",  1,	0xd8, 7, sld_FP|FloatMF|Modrm,	{ LongMem|LLongMem, 0, 0} },
{"fidivr", 1,	0xde, 7, sl_FP|FloatMF|Modrm,	{ ShortMem|LongMem, 0, 0} },

#if UNIXWARE_COMPAT
{"fdivrp", 2, 0xdef8, X, FP|ShortForm,		{ FloatAcc, FloatReg, 0} },
{"fdivrp", 1, 0xdef8, X, FP|ShortForm,		{ FloatReg, 0, 0} },
{"fdivrp", 0, 0xdef9, X, FP,			{ 0, 0, 0} },
{"fdivrp", 2, 0xdef8, X, FP|ShortForm|Ugh,	{ FloatReg, FloatAcc, 0} },
#else
{"fdivrp", 2, 0xdef0, X, FP|ShortForm,		{ FloatAcc, FloatReg, 0} },
{"fdivrp", 1, 0xdef0, X, FP|ShortForm,		{ FloatReg, 0, 0} },
{"fdivrp", 0, 0xdef1, X, FP,			{ 0, 0, 0} },
#endif

{"f2xm1",  0, 0xd9f0, X, FP,			{ 0, 0, 0} },
{"fyl2x",  0, 0xd9f1, X, FP,			{ 0, 0, 0} },
{"fptan",  0, 0xd9f2, X, FP,			{ 0, 0, 0} },
{"fpatan", 0, 0xd9f3, X, FP,			{ 0, 0, 0} },
{"fxtract",0, 0xd9f4, X, FP,			{ 0, 0, 0} },
{"fprem1", 0, 0xd9f5, X, FP,			{ 0, 0, 0} },
{"fdecstp",0, 0xd9f6, X, FP,			{ 0, 0, 0} },
{"fincstp",0, 0xd9f7, X, FP,			{ 0, 0, 0} },
{"fprem",  0, 0xd9f8, X, FP,			{ 0, 0, 0} },
{"fyl2xp1",0, 0xd9f9, X, FP,			{ 0, 0, 0} },
{"fsqrt",  0, 0xd9fa, X, FP,			{ 0, 0, 0} },
{"fsincos",0, 0xd9fb, X, FP,			{ 0, 0, 0} },
{"frndint",0, 0xd9fc, X, FP,			{ 0, 0, 0} },
{"fscale", 0, 0xd9fd, X, FP,			{ 0, 0, 0} },
{"fsin",   0, 0xd9fe, X, FP,			{ 0, 0, 0} },
{"fcos",   0, 0xd9ff, X, FP,			{ 0, 0, 0} },
{"fchs",   0, 0xd9e0, X, FP,			{ 0, 0, 0} },
{"fabs",   0, 0xd9e1, X, FP,			{ 0, 0, 0} },

/* processor control */
{"fninit", 0, 0xdbe3, X, FP,			{ 0, 0, 0} },
{"finit",  0, 0xdbe3, X, FP|FWait,		{ 0, 0, 0} },
{"fldcw",  1,	0xd9, 5, FP|Modrm,		{ ShortMem, 0, 0} },
{"fnstcw", 1,	0xd9, 7, FP|Modrm,		{ ShortMem, 0, 0} },
{"fstcw",  1,	0xd9, 7, FP|FWait|Modrm,	{ ShortMem, 0, 0} },
{"fnstsw", 1, 0xdfe0, X, FP,			{ Acc, 0, 0} },
{"fnstsw", 1,	0xdd, 7, FP|Modrm,		{ ShortMem, 0, 0} },
{"fnstsw", 0, 0xdfe0, X, FP,			{ 0, 0, 0} },
{"fstsw",  1, 0xdfe0, X, FP|FWait,		{ Acc, 0, 0} },
{"fstsw",  1,	0xdd, 7, FP|FWait|Modrm,	{ ShortMem, 0, 0} },
{"fstsw",  0, 0xdfe0, X, FP|FWait,		{ 0, 0, 0} },
{"fnclex", 0, 0xdbe2, X, FP,			{ 0, 0, 0} },
{"fclex",  0, 0xdbe2, X, FP|FWait,		{ 0, 0, 0} },
/* Short forms of fldenv, fstenv use data size prefix.
   FIXME: Are these the right suffixes?  */
{"fnstenv",1,	0xd9, 6, sl_Suf|Modrm,		{ LLongMem, 0, 0} },
{"fstenv", 1,	0xd9, 6, sl_Suf|FWait|Modrm,	{ LLongMem, 0, 0} },
{"fldenv", 1,	0xd9, 4, sl_Suf|Modrm,		{ LLongMem, 0, 0} },
{"fnsave", 1,	0xdd, 6, sl_Suf|Modrm,		{ LLongMem, 0, 0} },
{"fsave",  1,	0xdd, 6, sl_Suf|FWait|Modrm,	{ LLongMem, 0, 0} },
{"frstor", 1,	0xdd, 4, sl_Suf|Modrm,		{ LLongMem, 0, 0} },

{"ffree",  1, 0xddc0, X, FP|ShortForm,		{ FloatReg, 0, 0} },
/* P6:free st(i), pop st */
{"ffreep", 1, 0xdfc0, X, FP|ShortForm,		{ FloatReg, 0, 0} },
{"fnop",   0, 0xd9d0, X, FP,			{ 0, 0, 0} },
#define FWAIT_OPCODE 0x9b
{"fwait",  0,	0x9b, X, FP,			{ 0, 0, 0} },

/*
  opcode prefixes; we allow them as seperate insns too
*/
#define ADDR_PREFIX_OPCODE 0x67
{"addr16", 0,	0x67, X, NoSuf|IsPrefix|Size16|IgnoreSize,	{ 0, 0, 0} },
{"addr32", 0,	0x67, X, NoSuf|IsPrefix|Size32|IgnoreSize,	{ 0, 0, 0} },
{"aword",  0,	0x67, X, NoSuf|IsPrefix|Size16|IgnoreSize,	{ 0, 0, 0} },
{"adword", 0,	0x67, X, NoSuf|IsPrefix|Size32|IgnoreSize,	{ 0, 0, 0} },
#define DATA_PREFIX_OPCODE 0x66
{"data16", 0,	0x66, X, NoSuf|IsPrefix|Size16|IgnoreSize,	{ 0, 0, 0} },
{"data32", 0,	0x66, X, NoSuf|IsPrefix|Size32|IgnoreSize,	{ 0, 0, 0} },
{"word",   0,	0x66, X, NoSuf|IsPrefix|Size16|IgnoreSize,	{ 0, 0, 0} },
{"dword",  0,	0x66, X, NoSuf|IsPrefix|Size32|IgnoreSize,	{ 0, 0, 0} },
#define LOCK_PREFIX_OPCODE 0xf0
{"lock",   0,	0xf0, X, NoSuf|IsPrefix,	{ 0, 0, 0} },
{"wait",   0,   0x9b, X, NoSuf|IsPrefix,	{ 0, 0, 0} },
#define CS_PREFIX_OPCODE 0x2e
{"cs",	   0,	0x2e, X, NoSuf|IsPrefix,	{ 0, 0, 0} },
#define DS_PREFIX_OPCODE 0x3e
{"ds",	   0,	0x3e, X, NoSuf|IsPrefix,	{ 0, 0, 0} },
#define ES_PREFIX_OPCODE 0x26
{"es",	   0,	0x26, X, NoSuf|IsPrefix,	{ 0, 0, 0} },
#define FS_PREFIX_OPCODE 0x64
{"fs",	   0,	0x64, X, NoSuf|IsPrefix,	{ 0, 0, 0} },
#define GS_PREFIX_OPCODE 0x65
{"gs",	   0,	0x65, X, NoSuf|IsPrefix,	{ 0, 0, 0} },
#define SS_PREFIX_OPCODE 0x36
{"ss",	   0,	0x36, X, NoSuf|IsPrefix,	{ 0, 0, 0} },
#define REPNE_PREFIX_OPCODE 0xf2
#define REPE_PREFIX_OPCODE  0xf3
{"rep",	   0,	0xf3, X, NoSuf|IsPrefix,	{ 0, 0, 0} },
{"repe",   0,	0xf3, X, NoSuf|IsPrefix,	{ 0, 0, 0} },
{"repz",   0,	0xf3, X, NoSuf|IsPrefix,	{ 0, 0, 0} },
{"repne",  0,	0xf2, X, NoSuf|IsPrefix,	{ 0, 0, 0} },
{"repnz",  0,	0xf2, X, NoSuf|IsPrefix,	{ 0, 0, 0} },

/* 486 extensions */

{"bswap",   1, 0x0fc8, X, NoSuf|ShortForm,	{ Reg32,0,0 } },
{"xadd",    2, 0x0fc0, X, bwl_Suf|W|Modrm,	{ Reg, Reg|AnyMem, 0 } },
{"cmpxchg", 2, 0x0fb0, X, bwl_Suf|W|Modrm,	{ Reg, Reg|AnyMem, 0 } },
{"invd",    0, 0x0f08, X, NoSuf,		{ 0, 0, 0} },
{"wbinvd",  0, 0x0f09, X, NoSuf,		{ 0, 0, 0} },
{"invlpg",  1, 0x0f01, 7, NoSuf|Modrm,		{ AnyMem, 0, 0} },

/* 586 and late 486 extensions */
{"cpuid",   0, 0x0fa2, X, NoSuf,		{ 0, 0, 0} },

/* Pentium extensions */
{"wrmsr",   0, 0x0f30, X, NoSuf,		{ 0, 0, 0} },
{"rdtsc",   0, 0x0f31, X, NoSuf,		{ 0, 0, 0} },
{"rdmsr",   0, 0x0f32, X, NoSuf,		{ 0, 0, 0} },
{"cmpxchg8b",1,0x0fc7, 1, NoSuf|Modrm,		{ LLongMem, 0, 0} },
{"sysenter", 0, 0x0f34, X, NoSuf,		{ 0, 0, 0} },
{"sysexit",  0, 0x0f35, X, NoSuf,		{ 0, 0, 0} },
{"fxsave",   1,	0x0fae, 0, FP|Modrm,		{ LLongMem, 0, 0} },
{"fxrstor",  1,	0x0fae, 1, FP|Modrm,		{ LLongMem, 0, 0} },

/* Pentium Pro extensions */
{"rdpmc",   0, 0x0f33, X, NoSuf,		{ 0, 0, 0} },

{"ud2",	    0, 0x0f0b, X, NoSuf,		{ 0, 0, 0} }, /* official undefined instr. */
{"ud2a",    0, 0x0f0b, X, NoSuf,		{ 0, 0, 0} }, /* alias for ud2 */
{"ud2b",    0, 0x0fb9, X, NoSuf,		{ 0, 0, 0} }, /* 2nd. official undefined instr. */

{"cmovo",   2, 0x0f40, X, wl_Suf|ReverseModrm,	{ WordReg|WordMem, WordReg, 0} },
{"cmovno",  2, 0x0f41, X, wl_Suf|ReverseModrm,	{ WordReg|WordMem, WordReg, 0} },
{"cmovb",   2, 0x0f42, X, wl_Suf|ReverseModrm,	{ WordReg|WordMem, WordReg, 0} },
{"cmovc",   2, 0x0f42, X, wl_Suf|ReverseModrm,	{ WordReg|WordMem, WordReg, 0} },
{"cmovnae", 2, 0x0f42, X, wl_Suf|ReverseModrm,	{ WordReg|WordMem, WordReg, 0} },
{"cmovae",  2, 0x0f43, X, wl_Suf|ReverseModrm,	{ WordReg|WordMem, WordReg, 0} },
{"cmovnc",  2, 0x0f43, X, wl_Suf|ReverseModrm,	{ WordReg|WordMem, WordReg, 0} },
{"cmovnb",  2, 0x0f43, X, wl_Suf|ReverseModrm,	{ WordReg|WordMem, WordReg, 0} },
{"cmove",   2, 0x0f44, X, wl_Suf|ReverseModrm,	{ WordReg|WordMem, WordReg, 0} },
{"cmovz",   2, 0x0f44, X, wl_Suf|ReverseModrm,	{ WordReg|WordMem, WordReg, 0} },
{"cmovne",  2, 0x0f45, X, wl_Suf|ReverseModrm,	{ WordReg|WordMem, WordReg, 0} },
{"cmovnz",  2, 0x0f45, X, wl_Suf|ReverseModrm,	{ WordReg|WordMem, WordReg, 0} },
{"cmovbe",  2, 0x0f46, X, wl_Suf|ReverseModrm,	{ WordReg|WordMem, WordReg, 0} },
{"cmovna",  2, 0x0f46, X, wl_Suf|ReverseModrm,	{ WordReg|WordMem, WordReg, 0} },
{"cmova",   2, 0x0f47, X, wl_Suf|ReverseModrm,	{ WordReg|WordMem, WordReg, 0} },
{"cmovnbe", 2, 0x0f47, X, wl_Suf|ReverseModrm,	{ WordReg|WordMem, WordReg, 0} },
{"cmovs",   2, 0x0f48, X, wl_Suf|ReverseModrm,	{ WordReg|WordMem, WordReg, 0} },
{"cmovns",  2, 0x0f49, X, wl_Suf|ReverseModrm,	{ WordReg|WordMem, WordReg, 0} },
{"cmovp",   2, 0x0f4a, X, wl_Suf|ReverseModrm,	{ WordReg|WordMem, WordReg, 0} },
{"cmovnp",  2, 0x0f4b, X, wl_Suf|ReverseModrm,	{ WordReg|WordMem, WordReg, 0} },
{"cmovl",   2, 0x0f4c, X, wl_Suf|ReverseModrm,	{ WordReg|WordMem, WordReg, 0} },
{"cmovnge", 2, 0x0f4c, X, wl_Suf|ReverseModrm,	{ WordReg|WordMem, WordReg, 0} },
{"cmovge",  2, 0x0f4d, X, wl_Suf|ReverseModrm,	{ WordReg|WordMem, WordReg, 0} },
{"cmovnl",  2, 0x0f4d, X, wl_Suf|ReverseModrm,	{ WordReg|WordMem, WordReg, 0} },
{"cmovle",  2, 0x0f4e, X, wl_Suf|ReverseModrm,	{ WordReg|WordMem, WordReg, 0} },
{"cmovng",  2, 0x0f4e, X, wl_Suf|ReverseModrm,	{ WordReg|WordMem, WordReg, 0} },
{"cmovg",   2, 0x0f4f, X, wl_Suf|ReverseModrm,	{ WordReg|WordMem, WordReg, 0} },
{"cmovnle", 2, 0x0f4f, X, wl_Suf|ReverseModrm,	{ WordReg|WordMem, WordReg, 0} },

{"fcmovb",  2, 0xdac0, X, NoSuf|ShortForm,	{ FloatReg, FloatAcc, 0} },
{"fcmovnae",2, 0xdac0, X, NoSuf|ShortForm,	{ FloatReg, FloatAcc, 0} },
{"fcmove",  2, 0xdac8, X, NoSuf|ShortForm,	{ FloatReg, FloatAcc, 0} },
{"fcmovbe", 2, 0xdad0, X, NoSuf|ShortForm,	{ FloatReg, FloatAcc, 0} },
{"fcmovna", 2, 0xdad0, X, NoSuf|ShortForm,	{ FloatReg, FloatAcc, 0} },
{"fcmovu",  2, 0xdad8, X, NoSuf|ShortForm,	{ FloatReg, FloatAcc, 0} },
{"fcmovae", 2, 0xdbc0, X, NoSuf|ShortForm,	{ FloatReg, FloatAcc, 0} },
{"fcmovnb", 2, 0xdbc0, X, NoSuf|ShortForm,	{ FloatReg, FloatAcc, 0} },
{"fcmovne", 2, 0xdbc8, X, NoSuf|ShortForm,	{ FloatReg, FloatAcc, 0} },
{"fcmova",  2, 0xdbd0, X, NoSuf|ShortForm,	{ FloatReg, FloatAcc, 0} },
{"fcmovnbe",2, 0xdbd0, X, NoSuf|ShortForm,	{ FloatReg, FloatAcc, 0} },
{"fcmovnu", 2, 0xdbd8, X, NoSuf|ShortForm,	{ FloatReg, FloatAcc, 0} },

{"fcomi",   2, 0xdbf0, X, NoSuf|ShortForm,	{ FloatReg, FloatAcc, 0} },
{"fcomi",   0, 0xdbf1, X, NoSuf|ShortForm,	{ 0, 0, 0} },
{"fcomi",   1, 0xdbf0, X, NoSuf|ShortForm,	{ FloatReg, 0, 0} },
{"fucomi",  2, 0xdbe8, X, NoSuf|ShortForm,	{ FloatReg, FloatAcc, 0} },
{"fucomi",  0, 0xdbe9, X, NoSuf|ShortForm,	{ 0, 0, 0} },
{"fucomi",  1, 0xdbe8, X, NoSuf|ShortForm,	{ FloatReg, 0, 0} },
{"fcomip",  2, 0xdff0, X, NoSuf|ShortForm,	{ FloatReg, FloatAcc, 0} },
{"fcompi",  2, 0xdff0, X, NoSuf|ShortForm,	{ FloatReg, FloatAcc, 0} },
{"fcompi",  0, 0xdff1, X, NoSuf|ShortForm,	{ 0, 0, 0} },
{"fcompi",  1, 0xdff0, X, NoSuf|ShortForm,	{ FloatReg, 0, 0} },
{"fucomip", 2, 0xdfe8, X, NoSuf|ShortForm,	{ FloatReg, FloatAcc, 0} },
{"fucompi", 2, 0xdfe8, X, NoSuf|ShortForm,	{ FloatReg, FloatAcc, 0} },
{"fucompi", 0, 0xdfe9, X, NoSuf|ShortForm,	{ 0, 0, 0} },
{"fucompi", 1, 0xdfe8, X, NoSuf|ShortForm,	{ FloatReg, 0, 0} },

/* MMX instructions.  */

{"emms",     0, 0x0f77, X, NoSuf,		{ 0, 0, 0 } },
{"movd",     2, 0x0f6e, X, NoSuf|Modrm,		{ Reg32|LongMem, RegMMX, 0 } },
{"movd",     2, 0x0f7e, X, NoSuf|Modrm,		{ RegMMX, Reg32|LongMem, 0 } },
{"movq",     2, 0x0f6f, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"movq",     2, 0x0f7f, X, NoSuf|Modrm,		{ RegMMX, RegMMX|LongMem, 0 } },
{"packssdw", 2, 0x0f6b, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"packsswb", 2, 0x0f63, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"packuswb", 2, 0x0f67, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"paddb",    2, 0x0ffc, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"paddw",    2, 0x0ffd, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"paddd",    2, 0x0ffe, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"paddsb",   2, 0x0fec, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"paddsw",   2, 0x0fed, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"paddusb",  2, 0x0fdc, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"paddusw",  2, 0x0fdd, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"pand",     2, 0x0fdb, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"pandn",    2, 0x0fdf, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"pcmpeqb",  2, 0x0f74, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"pcmpeqw",  2, 0x0f75, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"pcmpeqd",  2, 0x0f76, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"pcmpgtb",  2, 0x0f64, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"pcmpgtw",  2, 0x0f65, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"pcmpgtd",  2, 0x0f66, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"pmaddwd",  2, 0x0ff5, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"pmulhw",   2, 0x0fe5, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"pmullw",   2, 0x0fd5, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"por",	     2, 0x0feb, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"psllw",    2, 0x0ff1, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"psllw",    2, 0x0f71, 6, NoSuf|Modrm,		{ Imm8, RegMMX, 0 } },
{"pslld",    2, 0x0ff2, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"pslld",    2, 0x0f72, 6, NoSuf|Modrm,		{ Imm8, RegMMX, 0 } },
{"psllq",    2, 0x0ff3, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"psllq",    2, 0x0f73, 6, NoSuf|Modrm,		{ Imm8, RegMMX, 0 } },
{"psraw",    2, 0x0fe1, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"psraw",    2, 0x0f71, 4, NoSuf|Modrm,		{ Imm8, RegMMX, 0 } },
{"psrad",    2, 0x0fe2, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"psrad",    2, 0x0f72, 4, NoSuf|Modrm,		{ Imm8, RegMMX, 0 } },
{"psrlw",    2, 0x0fd1, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"psrlw",    2, 0x0f71, 2, NoSuf|Modrm,		{ Imm8, RegMMX, 0 } },
{"psrld",    2, 0x0fd2, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"psrld",    2, 0x0f72, 2, NoSuf|Modrm,		{ Imm8, RegMMX, 0 } },
{"psrlq",    2, 0x0fd3, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"psrlq",    2, 0x0f73, 2, NoSuf|Modrm,		{ Imm8, RegMMX, 0 } },
{"psubb",    2, 0x0ff8, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"psubw",    2, 0x0ff9, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"psubd",    2, 0x0ffa, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"psubsb",   2, 0x0fe8, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"psubsw",   2, 0x0fe9, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"psubusb",  2, 0x0fd8, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"psubusw",  2, 0x0fd9, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"punpckhbw",2, 0x0f68, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"punpckhwd",2, 0x0f69, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"punpckhdq",2, 0x0f6a, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"punpcklbw",2, 0x0f60, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"punpcklwd",2, 0x0f61, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"punpckldq",2, 0x0f62, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },
{"pxor",     2, 0x0fef, X, NoSuf|Modrm,		{ RegMMX|LongMem, RegMMX, 0 } },

  
/* AMD 3DNow! instructions */
#define AMD_3DNOW_OPCODE 0x0f0f

{"prefetch", 1, 0x0f0d,	   0, NoSuf|Modrm,	{ ByteMem, 0, 0 } },
{"prefetchw",1, 0x0f0d,	   1, NoSuf|Modrm,	{ ByteMem, 0, 0 } },
{"femms",    0, 0x0f0e,	   X, NoSuf,		{ 0, 0, 0 } },
{"pavgusb",  2, 0x0f0f, 0xbf, NoSuf|Modrm,	{ RegMMX|LongMem, RegMMX, 0 } },
{"pf2id",    2, 0x0f0f, 0x1d, NoSuf|Modrm,	{ RegMMX|LongMem, RegMMX, 0 } },
{"pfacc",    2, 0x0f0f, 0xae, NoSuf|Modrm,	{ RegMMX|LongMem, RegMMX, 0 } },
{"pfadd",    2, 0x0f0f, 0x9e, NoSuf|Modrm,	{ RegMMX|LongMem, RegMMX, 0 } },
{"pfcmpeq",  2, 0x0f0f, 0xb0, NoSuf|Modrm,	{ RegMMX|LongMem, RegMMX, 0 } },
{"pfcmpge",  2, 0x0f0f, 0x90, NoSuf|Modrm,	{ RegMMX|LongMem, RegMMX, 0 } },
{"pfcmpgt",  2, 0x0f0f, 0xa0, NoSuf|Modrm,	{ RegMMX|LongMem, RegMMX, 0 } },
{"pfmax",    2, 0x0f0f, 0xa4, NoSuf|Modrm,	{ RegMMX|LongMem, RegMMX, 0 } },
{"pfmin",    2, 0x0f0f, 0x94, NoSuf|Modrm,	{ RegMMX|LongMem, RegMMX, 0 } },
{"pfmul",    2, 0x0f0f, 0xb4, NoSuf|Modrm,	{ RegMMX|LongMem, RegMMX, 0 } },
{"pfrcp",    2, 0x0f0f, 0x96, NoSuf|Modrm,	{ RegMMX|LongMem, RegMMX, 0 } },
{"pfrcpit1", 2, 0x0f0f, 0xa6, NoSuf|Modrm,	{ RegMMX|LongMem, RegMMX, 0 } },
{"pfrcpit2", 2, 0x0f0f, 0xb6, NoSuf|Modrm,	{ RegMMX|LongMem, RegMMX, 0 } },
{"pfrsqit1", 2, 0x0f0f, 0xa7, NoSuf|Modrm,	{ RegMMX|LongMem, RegMMX, 0 } },
{"pfrsqrt",  2, 0x0f0f, 0x97, NoSuf|Modrm,	{ RegMMX|LongMem, RegMMX, 0 } },
{"pfsub",    2, 0x0f0f, 0x9a, NoSuf|Modrm,	{ RegMMX|LongMem, RegMMX, 0 } },
{"pfsubr",   2, 0x0f0f, 0xaa, NoSuf|Modrm,	{ RegMMX|LongMem, RegMMX, 0 } },
{"pi2fd",    2, 0x0f0f, 0x0d, NoSuf|Modrm,	{ RegMMX|LongMem, RegMMX, 0 } },
{"pmulhrw",  2, 0x0f0f, 0xb7, NoSuf|Modrm,	{ RegMMX|LongMem, RegMMX, 0 } },

{NULL, 0, 0, 0, 0, { 0, 0, 0} }	/* sentinel */
};
#undef X
#undef ReverseModrm
#undef NoSuf
#undef b_Suf
#undef w_Suf
#undef l_Suf
#undef bw_Suf
#undef bl_Suf
#undef wl_Suf
#undef sl_Suf
#undef bwl_Suf
#undef FP
#undef l_FP
#undef sl_FP

#define MAX_MNEM_SIZE 16	/* for parsing insn mnemonics from input */


/* 386 register table */

static const reg_entry i386_regtab[] = {
  /* 8 bit regs */
  {"al", Reg8|Acc, 0},
  {"cl", Reg8|ShiftCount, 1},
  {"dl", Reg8, 2},
  {"bl", Reg8, 3},
  {"ah", Reg8, 4},
  {"ch", Reg8, 5},
  {"dh", Reg8, 6},
  {"bh", Reg8, 7},
  /* 16 bit regs */
  {"ax", Reg16|Acc, 0},
  {"cx", Reg16, 1},
  {"dx", Reg16|InOutPortReg, 2},
  {"bx", Reg16|BaseIndex, 3},
  {"sp", Reg16, 4},
  {"bp", Reg16|BaseIndex, 5},
  {"si", Reg16|BaseIndex, 6},
  {"di", Reg16|BaseIndex, 7},
  /* 32 bit regs */
  {"eax", Reg32|BaseIndex|Acc, 0},
  {"ecx", Reg32|BaseIndex, 1},
  {"edx", Reg32|BaseIndex, 2},
  {"ebx", Reg32|BaseIndex, 3},
  {"esp", Reg32, 4},
  {"ebp", Reg32|BaseIndex, 5},
  {"esi", Reg32|BaseIndex, 6},
  {"edi", Reg32|BaseIndex, 7},
  /* segment registers */
  {"es", SReg2, 0},
  {"cs", SReg2, 1},
  {"ss", SReg2, 2},
  {"ds", SReg2, 3},
  {"fs", SReg3, 4},
  {"gs", SReg3, 5},
  /* control registers */
  {"cr0", Control, 0},
  {"cr1", Control, 1},
  {"cr2", Control, 2},
  {"cr3", Control, 3},
  {"cr4", Control, 4},
  {"cr5", Control, 5},
  {"cr6", Control, 6},
  {"cr7", Control, 7},
  /* debug registers */
  {"db0", Debug, 0},
  {"db1", Debug, 1},
  {"db2", Debug, 2},
  {"db3", Debug, 3},
  {"db4", Debug, 4},
  {"db5", Debug, 5},
  {"db6", Debug, 6},
  {"db7", Debug, 7},
  {"dr0", Debug, 0},
  {"dr1", Debug, 1},
  {"dr2", Debug, 2},
  {"dr3", Debug, 3},
  {"dr4", Debug, 4},
  {"dr5", Debug, 5},
  {"dr6", Debug, 6},
  {"dr7", Debug, 7},
  /* test registers */
  {"tr0", Test, 0},
  {"tr1", Test, 1},
  {"tr2", Test, 2},
  {"tr3", Test, 3},
  {"tr4", Test, 4},
  {"tr5", Test, 5},
  {"tr6", Test, 6},
  {"tr7", Test, 7},
  /* float registers */
  {"st(0)", FloatReg|FloatAcc, 0},
  {"st", FloatReg|FloatAcc, 0},
  {"st(1)", FloatReg, 1},
  {"st(2)", FloatReg, 2},
  {"st(3)", FloatReg, 3},
  {"st(4)", FloatReg, 4},
  {"st(5)", FloatReg, 5},
  {"st(6)", FloatReg, 6},
  {"st(7)", FloatReg, 7},
  {"mm0", RegMMX, 0},
  {"mm1", RegMMX, 1},
  {"mm2", RegMMX, 2},
  {"mm3", RegMMX, 3},
  {"mm4", RegMMX, 4},
  {"mm5", RegMMX, 5},
  {"mm6", RegMMX, 6},
  {"mm7", RegMMX, 7}
};

#define MAX_REG_NAME_SIZE 8	/* for parsing register names from input */

/* segment stuff */
static const seg_entry cs = { "cs", 0x2e };
static const seg_entry ds = { "ds", 0x3e };
static const seg_entry ss = { "ss", 0x36 };
static const seg_entry es = { "es", 0x26 };
static const seg_entry fs = { "fs", 0x64 };
static const seg_entry gs = { "gs", 0x65 };

/* end of opcode/i386.h */
