/* sim-ppc.h --- interface between PowerPC simulator and GDB.

   Copyright 2004 Free Software Foundation, Inc.

   Contributed by Red Hat.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

#if !defined (SIM_PPC_H)
#define SIM_PPC_H


/* The register access functions, sim_fetch_register and
   sim_store_register, use the following numbering for PowerPC
   registers.  */

enum sim_ppc_regnum
  {
    /* General-purpose registers, r0 -- r31.  */
    sim_ppc_r0_regnum,
    sim_ppc_r1_regnum,
    sim_ppc_r2_regnum,
    sim_ppc_r3_regnum,
    sim_ppc_r4_regnum,
    sim_ppc_r5_regnum,
    sim_ppc_r6_regnum,
    sim_ppc_r7_regnum,
    sim_ppc_r8_regnum,
    sim_ppc_r9_regnum,
    sim_ppc_r10_regnum,
    sim_ppc_r11_regnum,
    sim_ppc_r12_regnum,
    sim_ppc_r13_regnum,
    sim_ppc_r14_regnum,
    sim_ppc_r15_regnum,
    sim_ppc_r16_regnum,
    sim_ppc_r17_regnum,
    sim_ppc_r18_regnum,
    sim_ppc_r19_regnum,
    sim_ppc_r20_regnum,
    sim_ppc_r21_regnum,
    sim_ppc_r22_regnum,
    sim_ppc_r23_regnum,
    sim_ppc_r24_regnum,
    sim_ppc_r25_regnum,
    sim_ppc_r26_regnum,
    sim_ppc_r27_regnum,
    sim_ppc_r28_regnum,
    sim_ppc_r29_regnum,
    sim_ppc_r30_regnum,
    sim_ppc_r31_regnum,

    /* Floating-point registers, f0 -- f31.  */
    sim_ppc_f0_regnum,
    sim_ppc_f1_regnum,
    sim_ppc_f2_regnum,
    sim_ppc_f3_regnum,
    sim_ppc_f4_regnum,
    sim_ppc_f5_regnum,
    sim_ppc_f6_regnum,
    sim_ppc_f7_regnum,
    sim_ppc_f8_regnum,
    sim_ppc_f9_regnum,
    sim_ppc_f10_regnum,
    sim_ppc_f11_regnum,
    sim_ppc_f12_regnum,
    sim_ppc_f13_regnum,
    sim_ppc_f14_regnum,
    sim_ppc_f15_regnum,
    sim_ppc_f16_regnum,
    sim_ppc_f17_regnum,
    sim_ppc_f18_regnum,
    sim_ppc_f19_regnum,
    sim_ppc_f20_regnum,
    sim_ppc_f21_regnum,
    sim_ppc_f22_regnum,
    sim_ppc_f23_regnum,
    sim_ppc_f24_regnum,
    sim_ppc_f25_regnum,
    sim_ppc_f26_regnum,
    sim_ppc_f27_regnum,
    sim_ppc_f28_regnum,
    sim_ppc_f29_regnum,
    sim_ppc_f30_regnum,
    sim_ppc_f31_regnum,

    /* Altivec vector registers, vr0 -- vr31.  */
    sim_ppc_vr0_regnum,
    sim_ppc_vr1_regnum,
    sim_ppc_vr2_regnum,
    sim_ppc_vr3_regnum,
    sim_ppc_vr4_regnum,
    sim_ppc_vr5_regnum,
    sim_ppc_vr6_regnum,
    sim_ppc_vr7_regnum,
    sim_ppc_vr8_regnum,
    sim_ppc_vr9_regnum,
    sim_ppc_vr10_regnum,
    sim_ppc_vr11_regnum,
    sim_ppc_vr12_regnum,
    sim_ppc_vr13_regnum,
    sim_ppc_vr14_regnum,
    sim_ppc_vr15_regnum,
    sim_ppc_vr16_regnum,
    sim_ppc_vr17_regnum,
    sim_ppc_vr18_regnum,
    sim_ppc_vr19_regnum,
    sim_ppc_vr20_regnum,
    sim_ppc_vr21_regnum,
    sim_ppc_vr22_regnum,
    sim_ppc_vr23_regnum,
    sim_ppc_vr24_regnum,
    sim_ppc_vr25_regnum,
    sim_ppc_vr26_regnum,
    sim_ppc_vr27_regnum,
    sim_ppc_vr28_regnum,
    sim_ppc_vr29_regnum,
    sim_ppc_vr30_regnum,
    sim_ppc_vr31_regnum,

    /* SPE APU GPR upper halves.  These are the upper 32 bits of the
       gprs; there is one upper-half register for each gpr, so it is
       appropriate to use sim_ppc_num_gprs for iterating through
       these.  */
    sim_ppc_rh0_regnum,
    sim_ppc_rh1_regnum,
    sim_ppc_rh2_regnum,
    sim_ppc_rh3_regnum,
    sim_ppc_rh4_regnum,
    sim_ppc_rh5_regnum,
    sim_ppc_rh6_regnum,
    sim_ppc_rh7_regnum,
    sim_ppc_rh8_regnum,
    sim_ppc_rh9_regnum,
    sim_ppc_rh10_regnum,
    sim_ppc_rh11_regnum,
    sim_ppc_rh12_regnum,
    sim_ppc_rh13_regnum,
    sim_ppc_rh14_regnum,
    sim_ppc_rh15_regnum,
    sim_ppc_rh16_regnum,
    sim_ppc_rh17_regnum,
    sim_ppc_rh18_regnum,
    sim_ppc_rh19_regnum,
    sim_ppc_rh20_regnum,
    sim_ppc_rh21_regnum,
    sim_ppc_rh22_regnum,
    sim_ppc_rh23_regnum,
    sim_ppc_rh24_regnum,
    sim_ppc_rh25_regnum,
    sim_ppc_rh26_regnum,
    sim_ppc_rh27_regnum,
    sim_ppc_rh28_regnum,
    sim_ppc_rh29_regnum,
    sim_ppc_rh30_regnum,
    sim_ppc_rh31_regnum,

    /* SPE APU GPR full registers.  Each of these registers is the
       64-bit concatenation of a 32-bit GPR (providing the lower bits)
       and a 32-bit upper-half register (providing the higher bits).
       As for the upper-half registers, it is appropriate to use
       sim_ppc_num_gprs with these.  */
    sim_ppc_ev0_regnum,
    sim_ppc_ev1_regnum,
    sim_ppc_ev2_regnum,
    sim_ppc_ev3_regnum,
    sim_ppc_ev4_regnum,
    sim_ppc_ev5_regnum,
    sim_ppc_ev6_regnum,
    sim_ppc_ev7_regnum,
    sim_ppc_ev8_regnum,
    sim_ppc_ev9_regnum,
    sim_ppc_ev10_regnum,
    sim_ppc_ev11_regnum,
    sim_ppc_ev12_regnum,
    sim_ppc_ev13_regnum,
    sim_ppc_ev14_regnum,
    sim_ppc_ev15_regnum,
    sim_ppc_ev16_regnum,
    sim_ppc_ev17_regnum,
    sim_ppc_ev18_regnum,
    sim_ppc_ev19_regnum,
    sim_ppc_ev20_regnum,
    sim_ppc_ev21_regnum,
    sim_ppc_ev22_regnum,
    sim_ppc_ev23_regnum,
    sim_ppc_ev24_regnum,
    sim_ppc_ev25_regnum,
    sim_ppc_ev26_regnum,
    sim_ppc_ev27_regnum,
    sim_ppc_ev28_regnum,
    sim_ppc_ev29_regnum,
    sim_ppc_ev30_regnum,
    sim_ppc_ev31_regnum,

    /* Segment registers, sr0 -- sr15.  */
    sim_ppc_sr0_regnum,
    sim_ppc_sr1_regnum,
    sim_ppc_sr2_regnum,
    sim_ppc_sr3_regnum,
    sim_ppc_sr4_regnum,
    sim_ppc_sr5_regnum,
    sim_ppc_sr6_regnum,
    sim_ppc_sr7_regnum,
    sim_ppc_sr8_regnum,
    sim_ppc_sr9_regnum,
    sim_ppc_sr10_regnum,
    sim_ppc_sr11_regnum,
    sim_ppc_sr12_regnum,
    sim_ppc_sr13_regnum,
    sim_ppc_sr14_regnum,
    sim_ppc_sr15_regnum,

    /* Miscellaneous --- but non-SPR --- registers.  */
    sim_ppc_pc_regnum,
    sim_ppc_ps_regnum,
    sim_ppc_cr_regnum,
    sim_ppc_fpscr_regnum,
    sim_ppc_acc_regnum,
    sim_ppc_vscr_regnum,

    /* Special-purpose registers.  Each SPR is given a number equal to
       its number in the ISA --- the number that appears in the mtspr
       / mfspr instructions --- plus this constant.  */
    sim_ppc_spr0_regnum
  };


/* Sizes of various register sets.  */
enum
  {
    sim_ppc_num_gprs = 32,
    sim_ppc_num_fprs = 32,
    sim_ppc_num_vrs = 32,
    sim_ppc_num_srs = 16,
    sim_ppc_num_sprs = 1024,
  };

#endif /* SIM_PPC_H */
