/* TI C6X instruction format information.
   Copyright 2010
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

/* Define the FMT macro before including this file; it takes a name
   and the fields from tic6x_insn_format (defined in tic6x.h).  */

#define FLD(name, pos, width) { CONCAT2(tic6x_field_,name), (pos), (width) }
#define CFLDS FLD(p, 0, 1), FLD(creg, 29, 3), FLD(z, 28, 1)
#define CFLDS2(a, b) 5, { CFLDS, a, b }
#define CFLDS3(a, b, c) 6, { CFLDS, a, b, c }
#define CFLDS4(a, b, c, d) 7, { CFLDS, a, b, c, d }
#define CFLDS5(a, b, c, d, e) 8, { CFLDS, a, b, c, d, e }
#define CFLDS6(a, b, c, d, e, f) 9, { CFLDS, a, b, c, d, e, f }
#define CFLDS7(a, b, c, d, e, f, g) 10, { CFLDS, a, b, c, d, e, f, g }
#define CFLDS8(a, b, c, d, e, f, g, h) 11, { CFLDS, a, b, c, d, e, f, g, h }
#define NFLDS FLD(p, 0, 1)
#define NFLDS1(a) 2, { NFLDS, a }
#define NFLDS2(a, b) 3, { NFLDS, a, b }
#define NFLDS3(a, b, c) 4, { NFLDS, a, b, c }
#define NFLDS5(a, b, c, d, e) 6, { NFLDS, a, b, c, d, e }
#define NFLDS6(a, b, c, d, e, f) 7, { NFLDS, a, b, c, d, e, f }
#define NFLDS7(a, b, c, d, e, f, g) 8, { NFLDS, a, b, c, d, e, f, g }

/* These are in the order from SPRUFE8, appendices C-H.  */

/* Appendix C 32-bit formats.  */

FMT(d_1_or_2_src, 32, 0x40, 0x7c,
    CFLDS5(FLD(s, 1, 1), FLD(op, 7, 6), FLD(src1, 13, 5), FLD(src2, 18, 5),
	   FLD(dst, 23, 5)))
FMT(d_ext_1_or_2_src, 32, 0x830, 0xc3c,
    CFLDS6(FLD(s, 1, 1), FLD(op, 6, 4), FLD(x, 12, 1), FLD(src1, 13, 5),
	   FLD(src2, 18, 5), FLD(dst, 23, 5)))
FMT(d_load_store, 32, 0x4, 0xc,
    CFLDS8(FLD(s, 1, 1), FLD(op, 4, 3), FLD(y, 7, 1), FLD(r, 8, 1),
	   FLD(mode, 9, 4), FLD(offsetR, 13, 5), FLD(baseR, 18, 5),
	   FLD(srcdst, 23, 5)))
/* The nonaligned loads and stores have the formats shown in the
   individual instruction descriptions; the appendix is incorrect.  */
FMT(d_load_nonaligned, 32, 0x124, 0x17c,
    CFLDS7(FLD(s, 1, 1), FLD(y, 7, 1), FLD(mode, 9, 4), FLD(offsetR, 13, 5),
	   FLD(baseR, 18, 5), FLD(sc, 23, 1), FLD(dst, 24, 4)))
FMT(d_store_nonaligned, 32, 0x174, 0x17c,
    CFLDS7(FLD(s, 1, 1), FLD(y, 7, 1), FLD(mode, 9, 4), FLD(offsetR, 13, 5),
	   FLD(baseR, 18, 5), FLD(sc, 23, 1), FLD(src, 24, 4)))
FMT(d_load_store_long, 32, 0xc, 0xc,
    CFLDS5(FLD(s, 1, 1), FLD(op, 4, 3), FLD(y, 7, 1), FLD(offsetR, 8, 15),
	   FLD(dst, 23, 5)))
FMT(d_adda_long, 32, 0x1000000c, 0xf000000c,
    NFLDS5(FLD(s, 1, 1), FLD(op, 4, 3), FLD(y, 7, 1), FLD(offsetR, 8, 15),
	   FLD(dst, 23, 5)))

/* Appendix C 16-bit formats will go here.  */

/* Appendix D 32-bit formats.  */

FMT(l_1_or_2_src, 32, 0x18, 0x1c,
    CFLDS6(FLD(s, 1, 1), FLD(op, 5, 7), FLD(x, 12, 1), FLD(src1, 13, 5),
	   FLD(src2, 18, 5), FLD(dst, 23, 5)))
FMT(l_1_or_2_src_noncond, 32, 0x10000018, 0xf000001c,
    NFLDS6(FLD(s, 1, 1), FLD(op, 5, 7), FLD(x, 12, 1), FLD(src1, 13, 5),
	   FLD(src2, 18, 5), FLD(dst, 23, 5)))
FMT(l_unary, 32, 0x358, 0xffc,
    CFLDS5(FLD(s, 1, 1), FLD(x, 12, 1), FLD(op, 13, 5), FLD(src2, 18, 5),
	   FLD(dst, 23, 5)))

/* Appendix D 16-bit formats will go here.  */

/* Appendix E 32-bit formats.  */

FMT(m_compound, 32, 0x30, 0x83c,
    CFLDS6(FLD(s, 1, 1), FLD(op, 6, 5), FLD(x, 12, 1), FLD(src1, 13, 5),
	   FLD(src2, 18, 5), FLD(dst, 23, 5)))
FMT(m_1_or_2_src, 32, 0x10000030, 0xf000083c,
    NFLDS6(FLD(s, 1, 1), FLD(op, 6, 5), FLD(x, 12, 1), FLD(src1, 13, 5),
	   FLD(src2, 18, 5), FLD(dst, 23, 5)))
/* Contrary to SPRUFE8, this does have predicate fields.  */
FMT(m_unary, 32, 0xf0, 0xffc,
    CFLDS5(FLD(s, 1, 1), FLD(x, 12, 1), FLD(op, 13, 5), FLD(src2, 18, 5),
	   FLD(dst, 23, 5)))

/* M-unit formats missing from Appendix E.  */
FMT(m_mpy, 32, 0x0, 0x7c,
    CFLDS6(FLD(s, 1, 1), FLD(op, 7, 5), FLD(x, 12, 1), FLD(src1, 13, 5),
	   FLD(src2, 18, 5), FLD(dst, 23, 5)))

/* Appendix E 16-bit formats will go here.  */

/* Appendix F 32-bit formats.  */

FMT(s_1_or_2_src, 32, 0x20, 0x3c,
    CFLDS6(FLD(s, 1, 1), FLD(op, 6, 6), FLD(x, 12, 1), FLD(src1, 13, 5),
	   FLD(src2, 18, 5), FLD(dst, 23 ,5)))
FMT(s_ext_1_or_2_src, 32, 0xc30, 0xc3c,
    CFLDS6(FLD(s, 1, 1), FLD(op, 6, 4), FLD(x, 12, 1), FLD(src1, 13, 5),
	   FLD(src2, 18, 5), FLD(dst, 23, 5)))
FMT(s_ext_1_or_2_src_noncond, 32, 0xc30, 0xe0000c3c,
    NFLDS7(FLD(s, 1, 1), FLD(op, 6, 4), FLD(x, 12, 1), FLD(src1, 13, 5),
	   FLD(src2, 18, 5), FLD(dst, 23, 5), FLD(z, 28, 1)))
FMT(s_unary, 32, 0xf20, 0xffc,
    CFLDS5(FLD(s, 1, 1), FLD(x, 12, 1), FLD(op, 13, 5), FLD(src2, 18, 5),
	   FLD(dst, 23, 5)))
FMT(s_ext_branch_cond_imm, 32, 0x10, 0x7c,
    CFLDS2(FLD(s, 1, 1), FLD(cst, 7, 21)))
FMT(s_call_imm_nop, 32, 0x10, 0xe000007c,
    NFLDS3(FLD(s, 1, 1), FLD(cst, 7, 21), FLD(z, 28, 1)))
FMT(s_branch_nop_cst, 32, 0x120, 0x1ffc,
    CFLDS3(FLD(s, 1, 1), FLD(src1, 13, 3), FLD(src2, 16, 12)))
FMT(s_branch_nop_reg, 32, 0x800360, 0xf830ffc,
    CFLDS4(FLD(s, 1, 1), FLD(x, 12, 1), FLD(src1, 13, 3), FLD(src2, 18, 5)))
FMT(s_branch, 32, 0x360, 0xf83effc,
    CFLDS3(FLD(s, 1, 1), FLD(x, 12, 1), FLD(src2, 18, 5)))
FMT(s_mvk, 32, 0x28, 0x3c,
    CFLDS4(FLD(s, 1, 1), FLD(h, 6, 1), FLD(cst, 7, 16), FLD(dst, 23, 5)))
FMT(s_field, 32, 0x8, 0x3c,
    CFLDS6(FLD(s, 1, 1), FLD(op, 6, 2), FLD(cstb, 8, 5), FLD(csta, 13, 5),
	   FLD(src2, 18, 5), FLD(dst, 23, 5)))

/* S-unit formats missing from Appendix F.  */
FMT(s_addk, 32, 0x50, 0x7c,
    CFLDS3(FLD(s, 1, 1), FLD(cst, 7, 16), FLD(dst, 23, 5)))
FMT(s_addkpc, 32, 0x160, 0x1ffc,
    CFLDS4(FLD(s, 1, 1), FLD(src2, 13, 3), FLD(src1, 16, 7), FLD(dst, 23, 5)))
FMT(s_b_irp, 32, 0x1800e0, 0x7feffc,
    CFLDS3(FLD(s, 1, 1), FLD(x, 12, 1), FLD(dst, 23, 5)))
FMT(s_b_nrp, 32, 0x1c00e0, 0x7feffc,
    CFLDS3(FLD(s, 1, 1), FLD(x, 12, 1), FLD(dst, 23, 5)))
FMT(s_bdec, 32, 0x1020, 0x1ffc,
    CFLDS3(FLD(s, 1, 1), FLD(src, 13, 10), FLD(dst, 23, 5)))
FMT(s_bpos, 32, 0x20, 0x1ffc,
    CFLDS3(FLD(s, 1, 1), FLD(src, 13, 10), FLD(dst, 23, 5)))

/* Appendix F 16-bit formats will go here.  */

/* Appendix G 16-bit formats will go here.  */

/* Appendix H 32-bit formats.  */

FMT(nfu_loop_buffer, 32, 0x00020000, 0x00021ffc,
    CFLDS4(FLD(s, 1, 1), FLD(op, 13, 4), FLD(csta, 18, 5), FLD(cstb, 23, 5)))
/* Corrected relative to Appendix H.  */
FMT(nfu_nop_idle, 32, 0x00000000, 0xfffe1ffc,
    NFLDS2(FLD(s, 1, 1), FLD(op, 13, 4)))

/* No-unit formats missing from Appendix H (given the NOP and IDLE
   correction).  */
FMT(nfu_dint, 32, 0x10004000, 0xfffffffc,
    NFLDS1(FLD(s, 1, 1)))
FMT(nfu_rint, 32, 0x10006000, 0xfffffffc,
    NFLDS1(FLD(s, 1, 1)))
FMT(nfu_swe, 32, 0x10000000, 0xfffffffc,
    NFLDS1(FLD(s, 1, 1)))
FMT(nfu_swenr, 32, 0x10002000, 0xfffffffc,
    NFLDS1(FLD(s, 1, 1)))
/* Although formally covered by the loop buffer format, the fields in
   that format are not useful for all such instructions and not all
   instructions can be predicated.  */
FMT(nfu_spkernel, 32, 0x00034000, 0xf03ffffc,
    NFLDS2(FLD(s, 1, 1), FLD(fstgfcyc, 22, 6)))
FMT(nfu_spkernelr, 32, 0x00036000, 0xfffffffc,
    NFLDS1(FLD(s, 1, 1)))
FMT(nfu_spmask, 32, 0x00020000, 0xfc021ffc,
    NFLDS3(FLD(s, 1, 1), FLD(op, 13, 4), FLD(mask, 18, 8)))

/* Appendix H 16-bit formats will go here.  */

#undef FLD
#undef CFLDS
#undef CFLDS2
#undef CFLDS3
#undef CFLDS4
#undef CFLDS5
#undef CFLDS6
#undef CFLDS7
#undef CFLDS8
#undef NFLDS
#undef NFLDS1
#undef NFLDS2
#undef NFLDS3
#undef NFLDS5
#undef NFLDS6
#undef NFLDS7
