/* Table of opcodes for the Texas Instruments TMS320C[34]X family.

   Copyright (c) 2002 Free Software Foundation.
  
   Contributed by Michael P. Hayes (m.hayes@elec.canterbury.ac.nz)
   
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


/* FIXME:  Only allow floating point registers for floating point
   instructions.  Use another field in the instruction table?
   This field could also flag which instructions are valid for
   which architectures...
   e.g., OP_FP | OP_C40  or OP_C40_FP  */

#define IS_CPU_C3X(v) ((v) == 30 || (v) == 31 || (v) == 32)
#define IS_CPU_C4X(v) ((v) ==  0 || (v) == 40 || (v) == 44)

/* Define some bitfield extraction/insertion macros.  */
#define EXTR(inst, m, l)          ((inst) << (31 - (m)) >> (31 - ((m) - (l)))) 
#define EXTRU(inst, m, l)         EXTR ((unsigned long)(inst), (m), (l))
#define EXTRS(inst, m, l)         EXTR ((long)(inst), (m), (l))
#define INSERTU(inst, val, m, l)  (inst |= ((val) << (l))) 
#define INSERTS(inst, val, m, l)  INSERTU (inst, ((val) & ((1 << ((m) - (l) + 1)) - 1)), m, l)

/* Define register numbers.  */
typedef enum
  {
    REG_R0, REG_R1, REG_R2, REG_R3,
    REG_R4, REG_R5, REG_R6, REG_R7,
    REG_AR0, REG_AR1, REG_AR2, REG_AR3,
    REG_AR4, REG_AR5, REG_AR6, REG_AR7,
    REG_DP, REG_IR0, REG_IR1, REG_BK,
    REG_SP, REG_ST, REG_DIE, REG_IIE,
    REG_IIF, REG_RS, REG_RE, REG_RC,
    REG_R8, REG_R9, REG_R10, REG_R11,
    REG_IVTP, REG_TVTP
  }
c4x_reg_t;

/* Note that the actual register numbers for IVTP is 0 and TVTP is 1.  */

#define REG_IE REG_DIE		/* C3x only */
#define REG_IF REG_IIE		/* C3x only */
#define REG_IOF REG_IIF		/* C3x only */

#define C3X_REG_MAX REG_RC
#define C4X_REG_MAX REG_TVTP

/* Register table size including C4x expansion regs.  */
#define REG_TABLE_SIZE (C4X_REG_MAX + 1)

struct c4x_register
{
  char *        name;
  unsigned long regno;
};

typedef struct c4x_register c4x_register_t;

/* We could store register synonyms here.  */
static const c4x_register_t c3x_registers[] =
{
  {"f0",  REG_R0},
  {"r0",  REG_R0},
  {"f1",  REG_R1},
  {"r1",  REG_R1},
  {"f2",  REG_R2},
  {"r2",  REG_R2},
  {"f3",  REG_R3},
  {"r3",  REG_R3},
  {"f4",  REG_R4},
  {"r4",  REG_R4},
  {"f5",  REG_R5},
  {"r5",  REG_R5},
  {"f6",  REG_R6},
  {"r6",  REG_R6},
  {"f7",  REG_R7},
  {"r7",  REG_R7},
  {"ar0", REG_AR0},
  {"ar1", REG_AR1},
  {"ar2", REG_AR2},
  {"ar3", REG_AR3},
  {"ar4", REG_AR4},
  {"ar5", REG_AR5},
  {"ar6", REG_AR6},
  {"ar7", REG_AR7},
  {"dp",  REG_DP},
  {"ir0", REG_IR0},
  {"ir1", REG_IR1},
  {"bk",  REG_BK},
  {"sp",  REG_SP},
  {"st",  REG_ST},
  {"ie",  REG_IE},
  {"if",  REG_IF},
  {"iof", REG_IOF},
  {"rs",  REG_RS},
  {"re",  REG_RE},
  {"rc",  REG_RC},
  {"", 0}
};

const unsigned int c3x_num_registers = (((sizeof c3x_registers) / (sizeof c3x_registers[0])) - 1);

/* Define C4x registers in addition to C3x registers.  */
static const c4x_register_t c4x_registers[] =
{
  {"die", REG_DIE},		/* Clobbers C3x REG_IE */
  {"iie", REG_IIE},		/* Clobbers C3x REG_IF */
  {"iif", REG_IIF},		/* Clobbers C3x REG_IOF */
  {"f8",  REG_R8},
  {"r8",  REG_R8},
  {"f9",  REG_R9},
  {"r9",  REG_R9},
  {"f10", REG_R10},
  {"r10", REG_R10},
  {"f11", REG_R11},
  {"r11", REG_R11},
  {"ivtp", REG_IVTP},
  {"tvtp", REG_TVTP},
  {"", 0}
};

const unsigned int c4x_num_registers = (((sizeof c4x_registers) / (sizeof c4x_registers[0])) - 1);

/* Instruction template.  */
struct c4x_inst
{
  char *        name;
  unsigned long opcode;
  unsigned long opmask;
  char *        args;
};

typedef struct c4x_inst c4x_inst_t;

/* B  condition             16--20
   C  condition             23--27
   ,  required arg follows
   ;  optional arg follows
   General addressing modes
   *  indirect               0--15 
   #  direct (for ldp only)  0--15 
   @  direct                 0--15 
   F  short float immediate  0--15 
   Q  register               0--15 
   R  register              16--20 
   S  short int immediate    0--15 
   D  src and dst same reg
   Three operand addressing modes
   E  register               0--7 
   G  register               8--15 
   I  indirect(short)        0--7 
   J  indirect(short)        8--15 
   R  register              16--20
   W  short int (C4x)        0--7
   C  indirect(short) (C4x)  0--7
   O  indirect(short) (C4x)  8--15
   Parallel instruction addressing modes
   E  register               0--7 
   G  register               8--15
   I  indirect(short)        0--7 
   J  indirect(short)        8--15 
   K  register              19--21
   L  register              22--24
   M  register (R2,R3)      22--22
   N  register (R0,R1)      23--23
   Misc. addressing modes
   A  address register      22--24
   B  unsigned integer       0--23  (absolute on C3x, relative on C4x)
   P  displacement (PC Rel)  0--15 
   U  unsigned integer       0--15
   V  vector                 0--4   (C4x 0--8) 
   T  integer (C4x stik)    16--20
   Y  address reg (C4x)     16--20   
   X  expansion reg (C4x)    0--4
   Z  expansion reg (C4x)   16--20.  */

#define C4X_OPERANDS_MAX 7	/* Max number of operands for an inst.  */
#define C4X_NAME_MAX 16		/* Max number of chars in parallel name.  */

/* General (two) operand group.  */
#define G_F_r "F,R"
#define G_I_r "S,R"
#define G_L_r "U,R"
#define G_Q_r "*,R"
#define G_T_r "@,R"
#define G_r_r "Q;R"

/* Three operand group (Type 1 with missing third operand).  */
#define T_rr_ "E,G"
#define T_rS_ "E,J"
#define T_Sr_ "I,G"
#define T_SS_ "I,J"

/* Three operand group (Type 2 with missing third operand).  */
#define T_Jr_ "W,G"		/* C4x only */
#define T_rJ_ "G,W"		/* C4x only (commutative insns only) */
#define T_Rr_ "C,G"		/* C4x only */
#define T_rR_ "G,C"		/* C4x only (commutative insns only) */
#define T_JR_ "W,O"		/* C4x only */
#define T_RJ_ "O,W"		/* C4x only (commutative insns only) */
#define T_RR_ "C,O"		/* C4x only */

/* Three operand group (Type 1).  */
#define T_rrr "E,G;R"
#define T_Srr "E,J,R"
#define T_rSr "I,G;R"
#define T_SSr "I,J,R"

/* Three operand group (Type 2).  */
#define T_Jrr "W,G;R"		/* C4x only */
#define T_rJr "G,W,R"		/* C4x only (commutative insns only) */
#define T_Rrr "C,G;R"		/* C4x only */
#define T_rRr "G,C,R"		/* C4x only (commutative insns only) */
#define T_JRr "W,O,R"		/* C4x only */
#define T_RJr "O,W,R"		/* C4x only (commutative insns only) */
#define T_RRr "C,O,R"		/* C4x only */

/* Parallel group (store || op).  */
#define Q_rS_rSr "H,J|K,I,L"
#define Q_rS_Sr  "H,J|I,L"
#define Q_rS_Srr "H,J|I,K;L"

/* Parallel group (op || store).  */
#define P_rSr_rS "K,I,L|H,J"
#define P_Srr_rS "I,K;L|H,J"
#define P_rS_rS  "L,I|H,J"

/* Parallel group (load || load).  */
#define P_Sr_Sr "I,L|J,K"
#define Q_Sr_Sr "J,K|I,L"

/* Parallel group (store || store).  */
#define P_Sr_rS "I,L|H,J"
#define Q_rS_rS "H,J|L,I"

/* Parallel group (multiply || add/sub).  */
#define P_SSr_rrr "I,J,N|H,K;M"	/* 00 (User manual transposes I,J) */
#define P_Srr_rSr "J,K;N|H,I,M"	/* 01 */
#define P_rSr_rSr "K,J,N|H,I,M"	/* 01 */
#define P_rrr_SSr "H,K;N|I,J,M"	/* 10 (User manual transposes H,K) */
#define P_Srr_Srr "J,K;N|I,H;M"	/* 11 */
#define P_rSr_Srr "K,J,N|I,H;M"	/* 11 */

#define Q_rrr_SSr "H,K;M|I,J,N"	/* 00 (User manual transposes I,J) */
#define Q_rSr_Srr "H,I,M|J,K;N"	/* 01 */
#define Q_rSr_rSr "H,I,M|K,J,N"	/* 01 */
#define Q_SSr_rrr "I,J,M|H,K;N"	/* 10 (User manual transposes H,K) */
#define Q_Srr_Srr "I,H;M|J,K;N"	/* 11 */
#define Q_Srr_rSr "I,H;M|K,J,N"	/* 11 */

/* Define c3x opcodes for assembler and disassembler.  */
static const c4x_inst_t c3x_insts[] =
{
  /* Put synonyms after the desired forms in table so that they get
     overwritten in the lookup table.  The disassembler will thus
     print the `proper' mnemonics.  Note that the disassembler
     only decodes the 11 MSBs, so instructions like ldp @0x500 will
     be printed as ldiu 5, dp.  Note that with parallel instructions,
     the second part is executed before the first part, unless
     the sti1||sti2 form is used.  We also allow sti2||sti1
     which is equivalent to the default sti||sti form.

     Put most common forms first to speed up assembler.
     
     FIXME:  Add all the other parallel/load forms, like absf1_stf2
     Perhaps I should have used a few macros...especially with
     all the bloat after adding the C4x opcodes...too late now!  */

    /* Parallel instructions.  */
  { "absf_stf",     0xc8000000, 0xfe000000, P_Sr_rS },
  { "absi_sti",     0xca000000, 0xfe000000, P_Sr_rS },
  { "addf_mpyf",    0x80000000, 0xff000000, Q_rrr_SSr },
  { "addf_mpyf",    0x81000000, 0xff000000, Q_rSr_Srr },
  { "addf_mpyf",    0x81000000, 0xff000000, Q_rSr_rSr },
  { "addf_mpyf",    0x82000000, 0xff000000, Q_SSr_rrr },
  { "addf_mpyf",    0x83000000, 0xff000000, Q_Srr_Srr },
  { "addf_mpyf",    0x83000000, 0xff000000, Q_Srr_rSr },
  { "addf3_mpyf3",  0x80000000, 0xff000000, Q_rrr_SSr },
  { "addf3_mpyf3",  0x81000000, 0xff000000, Q_rSr_Srr },
  { "addf3_mpyf3",  0x81000000, 0xff000000, Q_rSr_rSr },
  { "addf3_mpyf3",  0x82000000, 0xff000000, Q_SSr_rrr },
  { "addf3_mpyf3",  0x83000000, 0xff000000, Q_Srr_Srr },
  { "addf3_mpyf3",  0x83000000, 0xff000000, Q_Srr_rSr },
  { "addf_stf",     0xcc000000, 0xfe000000, P_Srr_rS },
  { "addf_stf",     0xcc000000, 0xfe000000, P_rSr_rS },
  { "addf3_stf",    0xcc000000, 0xfe000000, P_Srr_rS },
  { "addf3_stf",    0xcc000000, 0xfe000000, P_rSr_rS },
  { "addi_mpyi",    0x88000000, 0xff000000, Q_rrr_SSr },
  { "addi_mpyi",    0x89000000, 0xff000000, Q_rSr_Srr },
  { "addi_mpyi",    0x89000000, 0xff000000, Q_rSr_rSr },
  { "addi_mpyi",    0x8a000000, 0xff000000, Q_SSr_rrr },
  { "addi_mpyi",    0x8b000000, 0xff000000, Q_Srr_Srr },
  { "addi3_mpyi3",  0x88000000, 0xff000000, Q_rrr_SSr },
  { "addi3_mpyi3",  0x89000000, 0xff000000, Q_rSr_Srr },
  { "addi3_mpyi3",  0x8a000000, 0xff000000, Q_SSr_rrr },
  { "addi3_mpyi3",  0x8b000000, 0xff000000, Q_Srr_Srr },
  { "addi3_mpyi3",  0x8b000000, 0xff000000, Q_Srr_rSr },
  { "addi_sti",     0xce000000, 0xfe000000, P_Srr_rS },
  { "addi_sti",     0xce000000, 0xfe000000, P_rSr_rS },
  { "addi3_sti",    0xce000000, 0xfe000000, P_Srr_rS },
  { "addi3_sti",    0xce000000, 0xfe000000, P_rSr_rS },
  { "and_sti",      0xd0000000, 0xfe000000, P_Srr_rS },
  { "and_sti",      0xd0000000, 0xfe000000, P_rSr_rS },
  { "and3_sti",     0xd0000000, 0xfe000000, P_Srr_rS },
  { "and3_sti",     0xd0000000, 0xfe000000, P_rSr_rS },
  { "ash_sti",      0xd2000000, 0xfe000000, P_rSr_rS },
  { "ash3_sti",     0xd2000000, 0xfe000000, P_rSr_rS },
  { "fix_sti",      0xd4000000, 0xfe000000, P_Sr_rS },
  { "float_stf",    0xd6000000, 0xfe000000, P_Sr_rS },
  { "ldf_ldf",      0xc4000000, 0xfe000000, P_Sr_Sr },
  { "ldf1_ldf2",    0xc4000000, 0xfe000000, Q_Sr_Sr }, /* synonym */
  { "ldf2_ldf1",    0xc4000000, 0xfe000000, P_Sr_Sr }, /* synonym */
  { "ldf_stf",      0xd8000000, 0xfe000000, P_Sr_rS },
  { "ldi_ldi",      0xc6000000, 0xfe000000, P_Sr_Sr },
  { "ldi1_ldi2",    0xc6000000, 0xfe000000, Q_Sr_Sr }, /* synonym  */
  { "ldi2_ldi1",    0xc6000000, 0xfe000000, P_Sr_Sr }, /* synonym  */
  { "ldi_sti",      0xda000000, 0xfe000000, P_Sr_rS },
  { "lsh_sti",      0xdc000000, 0xfe000000, P_rSr_rS },
  { "lsh3_sti",     0xdc000000, 0xfe000000, P_rSr_rS },
  { "mpyf_addf",    0x80000000, 0xff000000, P_SSr_rrr },
  { "mpyf_addf",    0x81000000, 0xff000000, P_Srr_rSr },
  { "mpyf_addf",    0x81000000, 0xff000000, P_rSr_rSr },
  { "mpyf_addf",    0x82000000, 0xff000000, P_rrr_SSr },
  { "mpyf_addf",    0x83000000, 0xff000000, P_Srr_Srr },
  { "mpyf_addf",    0x83000000, 0xff000000, P_rSr_Srr },
  { "mpyf3_addf3",  0x80000000, 0xff000000, P_SSr_rrr },
  { "mpyf3_addf3",  0x81000000, 0xff000000, P_Srr_rSr },
  { "mpyf3_addf3",  0x81000000, 0xff000000, P_rSr_rSr },
  { "mpyf3_addf3",  0x82000000, 0xff000000, P_rrr_SSr },
  { "mpyf3_addf3",  0x83000000, 0xff000000, P_Srr_Srr },
  { "mpyf3_addf3",  0x83000000, 0xff000000, P_rSr_Srr },
  { "mpyf_stf",     0xde000000, 0xfe000000, P_Srr_rS },
  { "mpyf_stf",     0xde000000, 0xfe000000, P_rSr_rS },
  { "mpyf3_stf",    0xde000000, 0xfe000000, P_Srr_rS },
  { "mpyf3_stf",    0xde000000, 0xfe000000, P_rSr_rS },
  { "mpyf_subf",    0x84000000, 0xff000000, P_SSr_rrr },
  { "mpyf_subf",    0x85000000, 0xff000000, P_Srr_rSr },
  { "mpyf_subf",    0x85000000, 0xff000000, P_rSr_rSr },
  { "mpyf_subf",    0x86000000, 0xff000000, P_rrr_SSr },
  { "mpyf_subf",    0x87000000, 0xff000000, P_Srr_Srr },
  { "mpyf_subf",    0x87000000, 0xff000000, P_rSr_Srr },
  { "mpyf3_subf3",  0x84000000, 0xff000000, P_SSr_rrr },
  { "mpyf3_subf3",  0x85000000, 0xff000000, P_Srr_rSr },
  { "mpyf3_subf3",  0x85000000, 0xff000000, P_rSr_rSr },
  { "mpyf3_subf3",  0x86000000, 0xff000000, P_rrr_SSr },
  { "mpyf3_subf3",  0x87000000, 0xff000000, P_Srr_Srr },
  { "mpyf3_subf3",  0x87000000, 0xff000000, P_rSr_Srr },
  { "mpyi_addi",    0x88000000, 0xff000000, P_SSr_rrr },
  { "mpyi_addi",    0x89000000, 0xff000000, P_Srr_rSr },
  { "mpyi_addi",    0x89000000, 0xff000000, P_rSr_rSr },
  { "mpyi_addi",    0x8a000000, 0xff000000, P_rrr_SSr },
  { "mpyi_addi",    0x8b000000, 0xff000000, P_Srr_Srr },
  { "mpyi_addi",    0x8b000000, 0xff000000, P_rSr_Srr },
  { "mpyi3_addi3",  0x88000000, 0xff000000, P_SSr_rrr },
  { "mpyi3_addi3",  0x89000000, 0xff000000, P_Srr_rSr },
  { "mpyi3_addi3",  0x89000000, 0xff000000, P_rSr_rSr },
  { "mpyi3_addi3",  0x8a000000, 0xff000000, P_rrr_SSr },
  { "mpyi3_addi3",  0x8b000000, 0xff000000, P_Srr_Srr },
  { "mpyi3_addi3",  0x8b000000, 0xff000000, P_rSr_Srr },
  { "mpyi_sti",     0xe0000000, 0xfe000000, P_Srr_rS },
  { "mpyi_sti",     0xe0000000, 0xfe000000, P_rSr_rS },
  { "mpyi3_sti",    0xe0000000, 0xfe000000, P_Srr_rS },
  { "mpyi3_sti",    0xe0000000, 0xfe000000, P_rSr_rS },
  { "mpyi_subi",    0x8c000000, 0xff000000, P_SSr_rrr },
  { "mpyi_subi",    0x8d000000, 0xff000000, P_Srr_rSr },
  { "mpyi_subi",    0x8d000000, 0xff000000, P_rSr_rSr },
  { "mpyi_subi",    0x8e000000, 0xff000000, P_rrr_SSr },
  { "mpyi_subi",    0x8f000000, 0xff000000, P_Srr_Srr },
  { "mpyi_subi",    0x8f000000, 0xff000000, P_rSr_Srr },
  { "mpyi3_subi3",  0x8c000000, 0xff000000, P_SSr_rrr },
  { "mpyi3_subi3",  0x8d000000, 0xff000000, P_Srr_rSr },
  { "mpyi3_subi3",  0x8d000000, 0xff000000, P_rSr_rSr },
  { "mpyi3_subi3",  0x8e000000, 0xff000000, P_rrr_SSr },
  { "mpyi3_subi3",  0x8f000000, 0xff000000, P_Srr_Srr },
  { "mpyi3_subi3",  0x8f000000, 0xff000000, P_rSr_Srr },
  { "negf_stf",     0xe2000000, 0xfe000000, P_Sr_rS },
  { "negi_sti",     0xe4000000, 0xfe000000, P_Sr_rS },
  { "not_sti",      0xe6000000, 0xfe000000, P_Sr_rS },
  { "or3_sti",      0xe8000000, 0xfe000000, P_Srr_rS },
  { "or3_sti",      0xe8000000, 0xfe000000, P_rSr_rS },
  { "stf_absf",     0xc8000000, 0xfe000000, Q_rS_Sr },
  { "stf_addf",     0xcc000000, 0xfe000000, Q_rS_Srr },
  { "stf_addf",     0xcc000000, 0xfe000000, Q_rS_rSr },
  { "stf_addf3",    0xcc000000, 0xfe000000, Q_rS_Srr },
  { "stf_addf3",    0xcc000000, 0xfe000000, Q_rS_rSr },
  { "stf_float",    0xd6000000, 0xfe000000, Q_rS_Sr },
  { "stf_mpyf",     0xde000000, 0xfe000000, Q_rS_Srr },
  { "stf_mpyf",     0xde000000, 0xfe000000, Q_rS_rSr },
  { "stf_mpyf3",    0xde000000, 0xfe000000, Q_rS_Srr },
  { "stf_mpyf3",    0xde000000, 0xfe000000, Q_rS_rSr },
  { "stf_negf",     0xe2000000, 0xfe000000, Q_rS_Sr },
  { "stf_stf",      0xc0000000, 0xfe000000, P_rS_rS },
  { "stf1_stf2",    0xc0000000, 0xfe000000, Q_rS_rS }, /* synonym */
  { "stf2_stf1",    0xc0000000, 0xfe000000, P_rS_rS }, /* synonym */
  { "stf_subf",     0xea000000, 0xfe000000, Q_rS_rSr },
  { "stf_subf3",    0xea000000, 0xfe000000, Q_rS_rSr },
  { "sti_absi",     0xca000000, 0xfe000000, Q_rS_Sr },
  { "sti_addi",     0xce000000, 0xfe000000, Q_rS_Srr },
  { "sti_addi",     0xce000000, 0xfe000000, Q_rS_rSr },
  { "sti_addi3",    0xce000000, 0xfe000000, Q_rS_Srr },
  { "sti_addi3",    0xce000000, 0xfe000000, Q_rS_rSr },
  { "sti_and",      0xd0000000, 0xfe000000, Q_rS_Srr },
  { "sti_and",      0xd0000000, 0xfe000000, Q_rS_rSr },
  { "sti_and3",     0xd0000000, 0xfe000000, Q_rS_Srr },
  { "sti_and3",     0xd0000000, 0xfe000000, Q_rS_rSr },
  { "sti_ash3",     0xd2000000, 0xfe000000, Q_rS_rSr },
  { "sti_fix",      0xd4000000, 0xfe000000, Q_rS_Sr },
  { "sti_ldi",      0xda000000, 0xfe000000, Q_rS_Sr },
  { "sti_lsh",      0xdc000000, 0xfe000000, Q_rS_rSr },
  { "sti_lsh3",     0xdc000000, 0xfe000000, Q_rS_rSr },
  { "sti_mpyi",     0xe0000000, 0xfe000000, Q_rS_Srr },
  { "sti_mpyi",     0xe0000000, 0xfe000000, Q_rS_rSr },
  { "sti_mpyi3",    0xe0000000, 0xfe000000, Q_rS_Srr },
  { "sti_mpyi3",    0xe0000000, 0xfe000000, Q_rS_rSr },
  { "sti_negi",     0xe4000000, 0xfe000000, Q_rS_Sr },
  { "sti_not",      0xe6000000, 0xfe000000, Q_rS_Sr },
  { "sti_or",       0xe8000000, 0xfe000000, Q_rS_Srr },
  { "sti_or",       0xe8000000, 0xfe000000, Q_rS_rSr },
  { "sti_or3",      0xe8000000, 0xfe000000, Q_rS_Srr },
  { "sti_or3",      0xe8000000, 0xfe000000, Q_rS_rSr },
  { "sti_sti",      0xc2000000, 0xfe000000, P_rS_rS },
  { "sti1_sti2",    0xc2000000, 0xfe000000, Q_rS_rS }, /* synonym */
  { "sti2_sti1",    0xc2000000, 0xfe000000, P_rS_rS }, /* synonym */
  { "sti_subi",     0xec000000, 0xfe000000, Q_rS_rSr },    
  { "sti_subi3",    0xec000000, 0xfe000000, Q_rS_rSr },
  { "sti_xor",      0xee000000, 0xfe000000, Q_rS_Srr },
  { "sti_xor",      0xee000000, 0xfe000000, Q_rS_rSr },
  { "sti_xor3",     0xee000000, 0xfe000000, Q_rS_Srr },
  { "sti_xor3",     0xee000000, 0xfe000000, Q_rS_rSr },
  { "subf_mpyf",    0x84000000, 0xff000000, Q_rrr_SSr },
  { "subf_mpyf",    0x85000000, 0xff000000, Q_rSr_Srr },
  { "subf_mpyf",    0x85000000, 0xff000000, Q_rSr_rSr },
  { "subf_mpyf",    0x86000000, 0xff000000, Q_SSr_rrr },
  { "subf_mpyf",    0x87000000, 0xff000000, Q_Srr_Srr },
  { "subf_mpyf",    0x87000000, 0xff000000, Q_Srr_rSr },
  { "subf3_mpyf3",  0x84000000, 0xff000000, Q_rrr_SSr },
  { "subf3_mpyf3",  0x85000000, 0xff000000, Q_rSr_Srr },
  { "subf3_mpyf3",  0x85000000, 0xff000000, Q_rSr_rSr },
  { "subf3_mpyf3",  0x86000000, 0xff000000, Q_SSr_rrr },
  { "subf3_mpyf3",  0x87000000, 0xff000000, Q_Srr_Srr },
  { "subf3_mpyf3",  0x87000000, 0xff000000, Q_Srr_rSr },
  { "subf_stf",     0xea000000, 0xfe000000, P_rSr_rS },
  { "subf3_stf",    0xea000000, 0xfe000000, P_rSr_rS },
  { "subi_mpyi",    0x8c000000, 0xff000000, Q_rrr_SSr },
  { "subi_mpyi",    0x8d000000, 0xff000000, Q_rSr_Srr },
  { "subi_mpyi",    0x8d000000, 0xff000000, Q_rSr_rSr },
  { "subi_mpyi",    0x8e000000, 0xff000000, Q_SSr_rrr },
  { "subi_mpyi",    0x8f000000, 0xff000000, Q_Srr_Srr },
  { "subi_mpyi",    0x8f000000, 0xff000000, Q_Srr_rSr },
  { "subi3_mpyi3",  0x8c000000, 0xff000000, Q_rrr_SSr },
  { "subi3_mpyi3",  0x8d000000, 0xff000000, Q_rSr_Srr },
  { "subi3_mpyi3",  0x8d000000, 0xff000000, Q_rSr_rSr },
  { "subi3_mpyi3",  0x8e000000, 0xff000000, Q_SSr_rrr },
  { "subi3_mpyi3",  0x8f000000, 0xff000000, Q_Srr_Srr },
  { "subi3_mpyi3",  0x8f000000, 0xff000000, Q_Srr_rSr },
  { "subi_sti",     0xec000000, 0xfe000000, P_rSr_rS },    
  { "subi3_sti",    0xec000000, 0xfe000000, P_rSr_rS },
  { "xor_sti",      0xee000000, 0xfe000000, P_Srr_rS },
  { "xor_sti",      0xee000000, 0xfe000000, P_rSr_rS },
  { "xor3_sti",     0xee000000, 0xfe000000, P_Srr_rS },
  { "xor3_sti",     0xee000000, 0xfe000000, P_rSr_rS },

  { "absf",   0x00000000, 0xffe00000, G_r_r },
  { "absf",   0x00200000, 0xffe00000, G_T_r },
  { "absf",   0x00400000, 0xffe00000, G_Q_r },
  { "absf",   0x00600000, 0xffe00000, G_F_r },
  { "absi",   0x00800000, 0xffe00000, G_r_r },
  { "absi",   0x00a00000, 0xffe00000, G_T_r },
  { "absi",   0x00c00000, 0xffe00000, G_Q_r },
  { "absi",   0x00e00000, 0xffe00000, G_I_r },
  { "addc",   0x01000000, 0xffe00000, G_r_r },
  { "addc",   0x01200000, 0xffe00000, G_T_r },
  { "addc",   0x01400000, 0xffe00000, G_Q_r },
  { "addc",   0x01600000, 0xffe00000, G_I_r },
  { "addc",   0x20000000, 0xffe00000, T_rrr },
  { "addc",   0x20200000, 0xffe00000, T_Srr },
  { "addc",   0x20400000, 0xffe00000, T_rSr },
  { "addc",   0x20600000, 0xffe00000, T_SSr },
  { "addc",   0x30000000, 0xffe00000, T_Jrr }, /* C4x */
  { "addc",   0x30000000, 0xffe00000, T_rJr }, /* C4x */
  { "addc",   0x30200000, 0xffe00000, T_rRr }, /* C4x */
  { "addc",   0x30200000, 0xffe00000, T_Rrr }, /* C4x */
  { "addc",   0x30400000, 0xffe00000, T_JRr }, /* C4x */
  { "addc",   0x30400000, 0xffe00000, T_RJr }, /* C4x */
  { "addc",   0x30600000, 0xffe00000, T_RRr }, /* C4x */
  { "addc3",  0x20000000, 0xffe00000, T_rrr },
  { "addc3",  0x20200000, 0xffe00000, T_Srr },
  { "addc3",  0x20400000, 0xffe00000, T_rSr },
  { "addc3",  0x20600000, 0xffe00000, T_SSr },
  { "addc3",  0x30000000, 0xffe00000, T_Jrr }, /* C4x */
  { "addc3",  0x30000000, 0xffe00000, T_rJr }, /* C4x */
  { "addc3",  0x30200000, 0xffe00000, T_rRr }, /* C4x */
  { "addc3",  0x30200000, 0xffe00000, T_Rrr }, /* C4x */
  { "addc3",  0x30400000, 0xffe00000, T_JRr }, /* C4x */
  { "addc3",  0x30400000, 0xffe00000, T_RJr }, /* C4x */
  { "addc3",  0x30600000, 0xffe00000, T_RRr }, /* C4x */
  { "addf",   0x01800000, 0xffe00000, G_r_r },
  { "addf",   0x01a00000, 0xffe00000, G_T_r },
  { "addf",   0x01c00000, 0xffe00000, G_Q_r },
  { "addf",   0x01e00000, 0xffe00000, G_F_r },
  { "addf",   0x20800000, 0xffe00000, T_rrr },
  { "addf",   0x20a00000, 0xffe00000, T_Srr },
  { "addf",   0x20c00000, 0xffe00000, T_rSr },
  { "addf",   0x20e00000, 0xffe00000, T_SSr },
  { "addf",   0x30800000, 0xffe00000, T_Jrr }, /* C4x */
  { "addf",   0x30800000, 0xffe00000, T_rJr }, /* C4x */
  { "addf",   0x30a00000, 0xffe00000, T_rRr }, /* C4x */
  { "addf",   0x30a00000, 0xffe00000, T_Rrr }, /* C4x */
  { "addf",   0x30c00000, 0xffe00000, T_JRr }, /* C4x */
  { "addf",   0x30c00000, 0xffe00000, T_RJr }, /* C4x */
  { "addf",   0x30e00000, 0xffe00000, T_RRr }, /* C4x */
  { "addf3",  0x20800000, 0xffe00000, T_rrr },
  { "addf3",  0x20a00000, 0xffe00000, T_Srr },
  { "addf3",  0x20c00000, 0xffe00000, T_rSr },
  { "addf3",  0x20e00000, 0xffe00000, T_SSr },
  { "addf3",  0x30800000, 0xffe00000, T_Jrr }, /* C4x */
  { "addf3",  0x30800000, 0xffe00000, T_rJr }, /* C4x */
  { "addf3",  0x30a00000, 0xffe00000, T_rRr }, /* C4x */
  { "addf3",  0x30a00000, 0xffe00000, T_Rrr }, /* C4x */
  { "addf3",  0x30c00000, 0xffe00000, T_JRr }, /* C4x */
  { "addf3",  0x30c00000, 0xffe00000, T_RJr }, /* C4x */
  { "addf3",  0x30e00000, 0xffe00000, T_RRr }, /* C4x */
  { "addi",   0x02000000, 0xffe00000, G_r_r },
  { "addi",   0x02200000, 0xffe00000, G_T_r },
  { "addi",   0x02400000, 0xffe00000, G_Q_r },
  { "addi",   0x02600000, 0xffe00000, G_I_r },
  { "addi",   0x21000000, 0xffe00000, T_rrr },
  { "addi",   0x21200000, 0xffe00000, T_Srr },
  { "addi",   0x21400000, 0xffe00000, T_rSr },
  { "addi",   0x21600000, 0xffe00000, T_SSr },
  { "addi",   0x31000000, 0xffe00000, T_Jrr }, /* C4x */
  { "addi",   0x31000000, 0xffe00000, T_rJr }, /* C4x */
  { "addi",   0x31200000, 0xffe00000, T_rRr }, /* C4x */
  { "addi",   0x31200000, 0xffe00000, T_Rrr }, /* C4x */
  { "addi",   0x31400000, 0xffe00000, T_JRr }, /* C4x */
  { "addi",   0x31400000, 0xffe00000, T_RJr }, /* C4x */
  { "addi",   0x31600000, 0xffe00000, T_RRr }, /* C4x */
  { "addi3",  0x21000000, 0xffe00000, T_rrr },
  { "addi3",  0x21200000, 0xffe00000, T_Srr },
  { "addi3",  0x21400000, 0xffe00000, T_rSr },
  { "addi3",  0x21600000, 0xffe00000, T_SSr },
  { "addi3",  0x31000000, 0xffe00000, T_Jrr }, /* C4x */
  { "addi3",  0x31000000, 0xffe00000, T_rJr }, /* C4x */
  { "addi3",  0x31200000, 0xffe00000, T_rRr }, /* C4x */
  { "addi3",  0x31200000, 0xffe00000, T_Rrr }, /* C4x */
  { "addi3",  0x31400000, 0xffe00000, T_JRr }, /* C4x */
  { "addi3",  0x31400000, 0xffe00000, T_RJr }, /* C4x */
  { "addi3",  0x31600000, 0xffe00000, T_RRr }, /* C4x */
  { "and",    0x02800000, 0xffe00000, G_r_r },
  { "and",    0x02a00000, 0xffe00000, G_T_r },
  { "and",    0x02c00000, 0xffe00000, G_Q_r },
  { "and",    0x02e00000, 0xffe00000, G_L_r },
  { "and",    0x21800000, 0xffe00000, T_rrr },
  { "and",    0x21a00000, 0xffe00000, T_Srr },
  { "and",    0x21c00000, 0xffe00000, T_rSr },
  { "and",    0x21e00000, 0xffe00000, T_SSr },
  { "and",    0x31800000, 0xffe00000, T_Jrr }, /* C4x */
  { "and",    0x31800000, 0xffe00000, T_rJr }, /* C4x */
  { "and",    0x31a00000, 0xffe00000, T_rRr }, /* C4x */
  { "and",    0x31a00000, 0xffe00000, T_Rrr }, /* C4x */
  { "and",    0x31c00000, 0xffe00000, T_JRr }, /* C4x */
  { "and",    0x31c00000, 0xffe00000, T_RJr }, /* C4x */
  { "and",    0x31e00000, 0xffe00000, T_RRr }, /* C4x */
  { "and3",   0x21800000, 0xffe00000, T_rrr },
  { "and3",   0x21a00000, 0xffe00000, T_Srr },
  { "and3",   0x21c00000, 0xffe00000, T_rSr },
  { "and3",   0x21e00000, 0xffe00000, T_SSr },
  { "and3",   0x31800000, 0xffe00000, T_Jrr }, /* C4x */
  { "and3",   0x31800000, 0xffe00000, T_rJr }, /* C4x */
  { "and3",   0x31a00000, 0xffe00000, T_rRr }, /* C4x */
  { "and3",   0x31a00000, 0xffe00000, T_Rrr }, /* C4x */
  { "and3",   0x31c00000, 0xffe00000, T_JRr }, /* C4x */
  { "and3",   0x31c00000, 0xffe00000, T_RJr }, /* C4x */
  { "and3",   0x31e00000, 0xffe00000, T_RRr }, /* C4x */
  { "andn",   0x03000000, 0xffe00000, G_r_r },
  { "andn",   0x03200000, 0xffe00000, G_T_r },
  { "andn",   0x03400000, 0xffe00000, G_Q_r },
  { "andn",   0x03600000, 0xffe00000, G_L_r },
  { "andn",   0x22000000, 0xffe00000, T_rrr },
  { "andn",   0x22200000, 0xffe00000, T_Srr },
  { "andn",   0x22400000, 0xffe00000, T_rSr },
  { "andn",   0x22600000, 0xffe00000, T_SSr },
  { "andn",   0x32000000, 0xffe00000, T_Jrr }, /* C4x */
  { "andn",   0x32200000, 0xffe00000, T_Rrr }, /* C4x */
  { "andn",   0x32400000, 0xffe00000, T_JRr }, /* C4x */
  { "andn",   0x32600000, 0xffe00000, T_RRr }, /* C4x */
  { "andn3",  0x22000000, 0xffe00000, T_rrr },
  { "andn3",  0x22200000, 0xffe00000, T_Srr },
  { "andn3",  0x22400000, 0xffe00000, T_rSr },
  { "andn3",  0x22600000, 0xffe00000, T_SSr },
  { "andn3",  0x32000000, 0xffe00000, T_Jrr }, /* C4x */
  { "andn3",  0x32200000, 0xffe00000, T_Rrr }, /* C4x */
  { "andn3",  0x32400000, 0xffe00000, T_JRr }, /* C4x */
  { "andn3",  0x32600000, 0xffe00000, T_RRr }, /* C4x */
  { "ash",    0x03800000, 0xffe00000, G_r_r },
  { "ash",    0x03a00000, 0xffe00000, G_T_r },
  { "ash",    0x03c00000, 0xffe00000, G_Q_r },
  { "ash",    0x03e00000, 0xffe00000, G_I_r },
  { "ash",    0x22800000, 0xffe00000, T_rrr },
  { "ash",    0x22a00000, 0xffe00000, T_Srr },
  { "ash",    0x22c00000, 0xffe00000, T_rSr },
  { "ash",    0x22e00000, 0xffe00000, T_SSr },
  { "ash",    0x32800000, 0xffe00000, T_Jrr }, /* C4x */
  { "ash",    0x32a00000, 0xffe00000, T_Rrr }, /* C4x */
  { "ash",    0x32c00000, 0xffe00000, T_JRr }, /* C4x */
  { "ash",    0x32e00000, 0xffe00000, T_RRr }, /* C4x */
  { "ash3",   0x22800000, 0xffe00000, T_rrr },
  { "ash3",   0x22a00000, 0xffe00000, T_Srr },
  { "ash3",   0x22c00000, 0xffe00000, T_rSr },
  { "ash3",   0x22e00000, 0xffe00000, T_SSr },
  { "ash3",   0x32800000, 0xffe00000, T_Jrr }, /* C4x */
  { "ash3",   0x32a00000, 0xffe00000, T_Rrr }, /* C4x */
  { "ash3",   0x32c00000, 0xffe00000, T_JRr }, /* C4x */
  { "ash3",   0x32e00000, 0xffe00000, T_RRr }, /* C4x */
  { "bB",     0x68000000, 0xffe00000, "Q" },
  { "bB",     0x6a000000, 0xffe00000, "P" }, 
  { "b",      0x68000000, 0xffe00000, "Q" }, /* synonym for bu */
  { "b",      0x6a000000, 0xffe00000, "P" }, /* synonym for bu */
  { "bBd",    0x68200000, 0xffe00000, "Q" },
  { "bBd",    0x6a200000, 0xffe00000, "P" },
  { "bd",     0x68200000, 0xffe00000, "Q" }, /* synonym for bud */
  { "bd",     0x6a200000, 0xffe00000, "P" }, /* synonym for bud */
  { "br",     0x60000000, 0xff000000, "B" },
  { "brd",    0x61000000, 0xff000000, "B" },
  { "call",   0x62000000, 0xff000000, "B" },
  { "callB",  0x70000000, 0xffe00000, "Q" },
  { "callB",  0x72000000, 0xffe00000, "P" },
  { "cmpf",   0x04000000, 0xffe00000, G_r_r },
  { "cmpf",   0x04200000, 0xffe00000, G_T_r },
  { "cmpf",   0x04400000, 0xffe00000, G_Q_r },
  { "cmpf",   0x04600000, 0xffe00000, G_F_r },
  { "cmpf",   0x23000000, 0xffe00000, T_rr_ },
  { "cmpf",   0x23200000, 0xffe00000, T_rS_ },
  { "cmpf",   0x23400000, 0xffe00000, T_Sr_ },
  { "cmpf",   0x23600000, 0xffe00000, T_SS_ },
  { "cmpf",   0x33200000, 0xffe00000, T_Rr_ }, /* C4x */
  { "cmpf",   0x33600000, 0xffe00000, T_RR_ }, /* C4x */
  { "cmpf3",  0x23000000, 0xffe00000, T_rr_ },
  { "cmpf3",  0x23200000, 0xffe00000, T_rS_ },
  { "cmpf3",  0x23400000, 0xffe00000, T_Sr_ },
  { "cmpf3",  0x23600000, 0xffe00000, T_SS_ },
  { "cmpf3",  0x33200000, 0xffe00000, T_Rr_ }, /* C4x */
  { "cmpf3",  0x33600000, 0xffe00000, T_RR_ }, /* C4x */
  { "cmpi",   0x04800000, 0xffe00000, G_r_r },
  { "cmpi",   0x04a00000, 0xffe00000, G_T_r },
  { "cmpi",   0x04c00000, 0xffe00000, G_Q_r },
  { "cmpi",   0x04e00000, 0xffe00000, G_I_r },
  { "cmpi",   0x23800000, 0xffe00000, T_rr_ },
  { "cmpi",   0x23a00000, 0xffe00000, T_rS_ },
  { "cmpi",   0x23c00000, 0xffe00000, T_Sr_ },
  { "cmpi",   0x23e00000, 0xffe00000, T_SS_ },
  { "cmpi",   0x33800000, 0xffe00000, T_Jr_ }, /* C4x */
  { "cmpi",   0x33a00000, 0xffe00000, T_Rr_ }, /* C4x */
  { "cmpi",   0x33c00000, 0xffe00000, T_JR_ }, /* C4x */
  { "cmpi",   0x33e00000, 0xffe00000, T_RR_ }, /* C4x */
  { "cmpi3",  0x23800000, 0xffe00000, T_rr_ },
  { "cmpi3",  0x23a00000, 0xffe00000, T_rS_ },
  { "cmpi3",  0x23c00000, 0xffe00000, T_Sr_ },
  { "cmpi3",  0x23e00000, 0xffe00000, T_SS_ },
  { "cmpi3",  0x33800000, 0xffe00000, T_Jr_ }, /* C4x */
  { "cmpi3",  0x33a00000, 0xffe00000, T_Rr_ }, /* C4x */
  { "cmpi3",  0x33c00000, 0xffe00000, T_JR_ }, /* C4x */
  { "cmpi3",  0x33e00000, 0xffe00000, T_RR_ }, /* C4x */
  { "dbB",    0x6c000000, 0xfe200000, "A,Q" },
  { "dbB",    0x6e000000, 0xfe200000, "A,P" },
  { "db",     0x6c000000, 0xfe200000, "A,Q" }, /* synonym for dbu */
  { "db",     0x6e000000, 0xfe200000, "A,P" }, /* synonym for dbu */
  { "dbBd",   0x6c200000, 0xfe200000, "A,Q" },
  { "dbBd",   0x6e200000, 0xfe200000, "A,P" },
  { "dbd",    0x6c200000, 0xfe200000, "A,Q" }, /* synonym for dbud */
  { "dbd",    0x6e200000, 0xfe200000, "A,P" }, /* synonym for dbud */
  { "fix",    0x05000000, 0xffe00000, G_r_r },
  { "fix",    0x05200000, 0xffe00000, G_T_r },
  { "fix",    0x05400000, 0xffe00000, G_Q_r },
  { "fix",    0x05600000, 0xffe00000, G_F_r },
  { "float",  0x05800000, 0xffe00000, G_r_r },
  { "float",  0x05a00000, 0xffe00000, G_T_r },
  { "float",  0x05c00000, 0xffe00000, G_Q_r },
  { "float",  0x05e00000, 0xffe00000, G_I_r },
  { "iack",   0x1b200000, 0xffe00000, "@" },
  { "iack",   0x1b400000, 0xffe00000, "*" },
  { "idle",   0x06000000, 0xffffffff, "" },
  { "lde",    0x06800000, 0xffe00000, G_r_r },
  { "lde",    0x06a00000, 0xffe00000, G_T_r },
  { "lde",    0x06c00000, 0xffe00000, G_Q_r },
  { "lde",    0x06e00000, 0xffe00000, G_F_r },
  { "ldf",    0x07000000, 0xffe00000, G_r_r },
  { "ldf",    0x07200000, 0xffe00000, G_T_r },
  { "ldf",    0x07400000, 0xffe00000, G_Q_r },
  { "ldf",    0x07600000, 0xffe00000, G_F_r },
  { "ldfC",   0x40000000, 0xf0600000, G_r_r },
  { "ldfC",   0x40200000, 0xf0600000, G_T_r },
  { "ldfC",   0x40400000, 0xf0600000, G_Q_r },
  { "ldfC",   0x40600000, 0xf0600000, G_F_r },
  { "ldfi",   0x07a00000, 0xffe00000, G_T_r },
  { "ldfi",   0x07c00000, 0xffe00000, G_Q_r },
  { "ldi",    0x08000000, 0xffe00000, G_r_r },
  { "ldi",    0x08200000, 0xffe00000, G_T_r },
  { "ldi",    0x08400000, 0xffe00000, G_Q_r },
  { "ldi",    0x08600000, 0xffe00000, G_I_r },
  { "ldiC",   0x50000000, 0xf0600000, G_r_r },
  { "ldiC",   0x50200000, 0xf0600000, G_T_r },
  { "ldiC",   0x50400000, 0xf0600000, G_Q_r },
  { "ldiC",   0x50600000, 0xf0600000, G_I_r },
  { "ldii",   0x08a00000, 0xffe00000, G_T_r },
  { "ldii",   0x08c00000, 0xffe00000, G_Q_r },
  { "ldp",    0x50700000, 0xffff0000, "#" }, /* synonym for ldiu #,dp */
  { "ldm",    0x09000000, 0xffe00000, G_r_r },
  { "ldm",    0x09200000, 0xffe00000, G_T_r },
  { "ldm",    0x09400000, 0xffe00000, G_Q_r },
  { "ldm",    0x09600000, 0xffe00000, G_F_r },
  { "lsh",    0x09800000, 0xffe00000, G_r_r },
  { "lsh",    0x09a00000, 0xffe00000, G_T_r },
  { "lsh",    0x09c00000, 0xffe00000, G_Q_r },
  { "lsh",    0x09e00000, 0xffe00000, G_I_r },
  { "lsh",    0x24000000, 0xffe00000, T_rrr },
  { "lsh",    0x24200000, 0xffe00000, T_Srr },
  { "lsh",    0x24400000, 0xffe00000, T_rSr },
  { "lsh",    0x24600000, 0xffe00000, T_SSr },
  { "lsh",    0x34000000, 0xffe00000, T_Jrr }, /* C4x */
  { "lsh",    0x34200000, 0xffe00000, T_Rrr }, /* C4x */
  { "lsh",    0x34400000, 0xffe00000, T_JRr }, /* C4x */
  { "lsh",    0x34600000, 0xffe00000, T_RRr }, /* C4x */
  { "lsh3",   0x24000000, 0xffe00000, T_rrr },
  { "lsh3",   0x24200000, 0xffe00000, T_Srr },
  { "lsh3",   0x24400000, 0xffe00000, T_rSr },
  { "lsh3",   0x24600000, 0xffe00000, T_SSr },
  { "lsh3",   0x34000000, 0xffe00000, T_Jrr }, /* C4x */
  { "lsh3",   0x34200000, 0xffe00000, T_Rrr }, /* C4x */
  { "lsh3",   0x34400000, 0xffe00000, T_JRr }, /* C4x */
  { "lsh3",   0x34600000, 0xffe00000, T_RRr }, /* C4x */
  { "mpyf",   0x0a000000, 0xffe00000, G_r_r },
  { "mpyf",   0x0a200000, 0xffe00000, G_T_r },
  { "mpyf",   0x0a400000, 0xffe00000, G_Q_r },
  { "mpyf",   0x0a600000, 0xffe00000, G_F_r },
  { "mpyf",   0x24800000, 0xffe00000, T_rrr },
  { "mpyf",   0x24a00000, 0xffe00000, T_Srr },
  { "mpyf",   0x24c00000, 0xffe00000, T_rSr },
  { "mpyf",   0x24e00000, 0xffe00000, T_SSr },
  { "mpyf",   0x34800000, 0xffe00000, T_Jrr }, /* C4x */
  { "mpyf",   0x34800000, 0xffe00000, T_rJr }, /* C4x */
  { "mpyf",   0x34a00000, 0xffe00000, T_rRr }, /* C4x */
  { "mpyf",   0x34a00000, 0xffe00000, T_Rrr }, /* C4x */
  { "mpyf",   0x34c00000, 0xffe00000, T_JRr }, /* C4x */
  { "mpyf",   0x34c00000, 0xffe00000, T_RJr }, /* C4x */
  { "mpyf",   0x34e00000, 0xffe00000, T_RRr }, /* C4x */
  { "mpyf3",  0x24800000, 0xffe00000, T_rrr },
  { "mpyf3",  0x24a00000, 0xffe00000, T_Srr },
  { "mpyf3",  0x24c00000, 0xffe00000, T_rSr },
  { "mpyf3",  0x24e00000, 0xffe00000, T_SSr },
  { "mpyf3",  0x34800000, 0xffe00000, T_Jrr }, /* C4x */
  { "mpyf3",  0x34800000, 0xffe00000, T_rJr }, /* C4x */
  { "mpyf3",  0x34a00000, 0xffe00000, T_rRr }, /* C4x */
  { "mpyf3",  0x34a00000, 0xffe00000, T_Rrr }, /* C4x */
  { "mpyf3",  0x34c00000, 0xffe00000, T_JRr }, /* C4x */
  { "mpyf3",  0x34c00000, 0xffe00000, T_RJr }, /* C4x */
  { "mpyf3",  0x34e00000, 0xffe00000, T_RRr }, /* C4x */
  { "mpyi",   0x0a800000, 0xffe00000, G_r_r },
  { "mpyi",   0x0aa00000, 0xffe00000, G_T_r },
  { "mpyi",   0x0ac00000, 0xffe00000, G_Q_r },
  { "mpyi",   0x0ae00000, 0xffe00000, G_I_r },
  { "mpyi",   0x25000000, 0xffe00000, T_rrr },
  { "mpyi",   0x25200000, 0xffe00000, T_Srr },
  { "mpyi",   0x25400000, 0xffe00000, T_rSr },
  { "mpyi",   0x25600000, 0xffe00000, T_SSr },
  { "mpyi",   0x35000000, 0xffe00000, T_Jrr }, /* C4x */
  { "mpyi",   0x35000000, 0xffe00000, T_rJr }, /* C4x */
  { "mpyi",   0x35200000, 0xffe00000, T_rRr }, /* C4x */
  { "mpyi",   0x35200000, 0xffe00000, T_Rrr }, /* C4x */
  { "mpyi",   0x35400000, 0xffe00000, T_JRr }, /* C4x */
  { "mpyi",   0x35400000, 0xffe00000, T_RJr }, /* C4x */
  { "mpyi",   0x35600000, 0xffe00000, T_RRr }, /* C4x */
  { "mpyi3",  0x25000000, 0xffe00000, T_rrr },
  { "mpyi3",  0x25200000, 0xffe00000, T_Srr },
  { "mpyi3",  0x25400000, 0xffe00000, T_rSr },
  { "mpyi3",  0x25600000, 0xffe00000, T_SSr },
  { "mpyi3",  0x35000000, 0xffe00000, T_Jrr }, /* C4x */
  { "mpyi3",  0x35000000, 0xffe00000, T_rJr }, /* C4x */
  { "mpyi3",  0x35200000, 0xffe00000, T_rRr }, /* C4x */
  { "mpyi3",  0x35200000, 0xffe00000, T_Rrr }, /* C4x */
  { "mpyi3",  0x35400000, 0xffe00000, T_JRr }, /* C4x */
  { "mpyi3",  0x35400000, 0xffe00000, T_RJr }, /* C4x */
  { "mpyi3",  0x35600000, 0xffe00000, T_RRr }, /* C4x */
  { "negb",   0x0b000000, 0xffe00000, G_r_r },
  { "negb",   0x0b200000, 0xffe00000, G_T_r },
  { "negb",   0x0b400000, 0xffe00000, G_Q_r },
  { "negb",   0x0b600000, 0xffe00000, G_I_r },
  { "negf",   0x0b800000, 0xffe00000, G_r_r },
  { "negf",   0x0ba00000, 0xffe00000, G_T_r },
  { "negf",   0x0bc00000, 0xffe00000, G_Q_r },
  { "negf",   0x0be00000, 0xffe00000, G_F_r },
  { "negi",   0x0c000000, 0xffe00000, G_r_r },
  { "negi",   0x0c200000, 0xffe00000, G_T_r },
  { "negi",   0x0c400000, 0xffe00000, G_Q_r },
  { "negi",   0x0c600000, 0xffe00000, G_I_r },
  { "nop",    0x0c800000, 0xffe00000, "Q" },
  { "nop",    0x0cc00000, 0xffe00000, "*" },
  { "nop",    0x0c800000, 0xffe00000, "" },
  { "norm",   0x0d000000, 0xffe00000, G_r_r },
  { "norm",   0x0d200000, 0xffe00000, G_T_r },
  { "norm",   0x0d400000, 0xffe00000, G_Q_r },
  { "norm",   0x0d600000, 0xffe00000, G_F_r },
  { "not",    0x0d800000, 0xffe00000, G_r_r },
  { "not",    0x0da00000, 0xffe00000, G_T_r },
  { "not",    0x0dc00000, 0xffe00000, G_Q_r },
  { "not",    0x0de00000, 0xffe00000, G_L_r },
  { "or",     0x10000000, 0xffe00000, G_r_r },
  { "or",     0x10200000, 0xffe00000, G_T_r },
  { "or",     0x10400000, 0xffe00000, G_Q_r },
  { "or",     0x10600000, 0xffe00000, G_L_r },
  { "or",     0x25800000, 0xffe00000, T_rrr },
  { "or",     0x25a00000, 0xffe00000, T_Srr },
  { "or",     0x25c00000, 0xffe00000, T_rSr },
  { "or",     0x25e00000, 0xffe00000, T_SSr },
  { "or",     0x35800000, 0xffe00000, T_Jrr }, /* C4x */
  { "or",     0x35800000, 0xffe00000, T_rJr }, /* C4x */
  { "or",     0x35a00000, 0xffe00000, T_rRr }, /* C4x */
  { "or",     0x35a00000, 0xffe00000, T_Rrr }, /* C4x */
  { "or",     0x35c00000, 0xffe00000, T_JRr }, /* C4x */
  { "or",     0x35c00000, 0xffe00000, T_RJr }, /* C4x */
  { "or",     0x35e00000, 0xffe00000, T_RRr }, /* C4x */
  { "or3",    0x25800000, 0xffe00000, T_rrr },
  { "or3",    0x25a00000, 0xffe00000, T_Srr },
  { "or3",    0x25c00000, 0xffe00000, T_rSr },
  { "or3",    0x25e00000, 0xffe00000, T_SSr },
  { "or3",    0x35800000, 0xffe00000, T_Jrr }, /* C4x */
  { "or3",    0x35800000, 0xffe00000, T_rJr }, /* C4x */
  { "or3",    0x35a00000, 0xffe00000, T_rRr }, /* C4x */
  { "or3",    0x35a00000, 0xffe00000, T_Rrr }, /* C4x */
  { "or3",    0x35c00000, 0xffe00000, T_JRr }, /* C4x */
  { "or3",    0x35c00000, 0xffe00000, T_RJr }, /* C4x */
  { "or3",    0x35e00000, 0xffe00000, T_RRr }, /* C4x */
  { "pop",    0x0e200000, 0xffe00000, "R" },
  { "popf",   0x0ea00000, 0xffe00000, "R" },
  { "push",   0x0f200000, 0xffe00000, "R" },
  { "pushf",  0x0fa00000, 0xffe00000, "R" },
  { "retiB",  0x78000000, 0xffe00000, "" },
  { "reti",   0x78000000, 0xffe00000, "" }, /* synonym for reti */
  { "retsB",  0x78800000, 0xffe00000, "" },
  { "rets",   0x78800000, 0xffe00000, "" }, /* synonym for rets */
  { "rnd",    0x11000000, 0xffe00000, G_r_r },
  { "rnd",    0x11200000, 0xffe00000, G_T_r },
  { "rnd",    0x11400000, 0xffe00000, G_Q_r },
  { "rnd",    0x11600000, 0xffe00000, G_F_r },
  { "rol",    0x11e00000, 0xffe00000, "R" },
  { "rolc",   0x12600000, 0xffe00000, "R" },
  { "ror",    0x12e00000, 0xffe00000, "R" },
  { "rorc",   0x13600000, 0xffe00000, "R" },
  { "rptb",   0x64000000, 0xff000000, "B" },
  { "rptb",   0x79000000, 0xff000000, "Q" }, /* C4x */
  { "rpts",   0x139b0000, 0xffff0000, "Q" },
  { "rpts",   0x13bb0000, 0xffff0000, "@" },
  { "rpts",   0x13db0000, 0xffff0000, "*" },
  { "rpts",   0x13fb0000, 0xffff0000, "U" },
  { "sigi",   0x16000000, 0xffe00000, "" },  /* C3x */
  { "sigi",   0x16200000, 0xffe00000, G_T_r }, /* C4x */
  { "sigi",   0x16400000, 0xffe00000, G_Q_r }, /* C4x */
  { "stf",    0x14200000, 0xffe00000, "R,@" },
  { "stf",    0x14400000, 0xffe00000, "R,*" },
  { "stfi",   0x14a00000, 0xffe00000, "R,@" },
  { "stfi",   0x14c00000, 0xffe00000, "R,*" },
  { "sti",    0x15000000, 0xffe00000, "T,@" }, /* C4x only */
  { "sti",    0x15200000, 0xffe00000, "R,@" },
  { "sti",    0x15400000, 0xffe00000, "R,*" },
  { "sti",    0x15600000, 0xffe00000, "T,*" }, /* C4x only */
  { "stii",   0x15a00000, 0xffe00000, "R,@" },
  { "stii",   0x15c00000, 0xffe00000, "R,*" },
  { "subb",   0x16800000, 0xffe00000, G_r_r },
  { "subb",   0x16a00000, 0xffe00000, G_T_r },
  { "subb",   0x16c00000, 0xffe00000, G_Q_r },
  { "subb",   0x16e00000, 0xffe00000, G_I_r },
  { "subb",   0x26000000, 0xffe00000, T_rrr },
  { "subb",   0x26200000, 0xffe00000, T_Srr },
  { "subb",   0x26400000, 0xffe00000, T_rSr },
  { "subb",   0x26600000, 0xffe00000, T_SSr },
  { "subb",   0x36000000, 0xffe00000, T_Jrr }, /* C4x */
  { "subb",   0x36200000, 0xffe00000, T_Rrr }, /* C4x */
  { "subb",   0x36400000, 0xffe00000, T_JRr }, /* C4x */
  { "subb",   0x36600000, 0xffe00000, T_RRr }, /* C4x */
  { "subb3",  0x26000000, 0xffe00000, T_rrr },
  { "subb3",  0x26200000, 0xffe00000, T_Srr },
  { "subb3",  0x26400000, 0xffe00000, T_rSr },
  { "subb3",  0x26600000, 0xffe00000, T_SSr },
  { "subb3",  0x36000000, 0xffe00000, T_Jrr }, /* C4x */
  { "subb3",  0x36200000, 0xffe00000, T_Rrr }, /* C4x */
  { "subb3",  0x36400000, 0xffe00000, T_JRr }, /* C4x */
  { "subb3",  0x36600000, 0xffe00000, T_RRr }, /* C4x */
  { "subc",   0x17000000, 0xffe00000, G_r_r },
  { "subc",   0x17200000, 0xffe00000, G_T_r },
  { "subc",   0x17400000, 0xffe00000, G_Q_r },
  { "subc",   0x17600000, 0xffe00000, G_I_r },
  { "subf",   0x17800000, 0xffe00000, G_r_r },
  { "subf",   0x17a00000, 0xffe00000, G_T_r },
  { "subf",   0x17c00000, 0xffe00000, G_Q_r },
  { "subf",   0x17e00000, 0xffe00000, G_F_r },
  { "subf",   0x26800000, 0xffe00000, T_rrr },
  { "subf",   0x26a00000, 0xffe00000, T_Srr },
  { "subf",   0x26c00000, 0xffe00000, T_rSr },
  { "subf",   0x26e00000, 0xffe00000, T_SSr },
  { "subf",   0x36800000, 0xffe00000, T_Jrr }, /* C4x */
  { "subf",   0x36a00000, 0xffe00000, T_Rrr }, /* C4x */
  { "subf",   0x36c00000, 0xffe00000, T_JRr }, /* C4x */
  { "subf",   0x36e00000, 0xffe00000, T_RRr }, /* C4x */
  { "subf3",  0x26800000, 0xffe00000, T_rrr },
  { "subf3",  0x26a00000, 0xffe00000, T_Srr },
  { "subf3",  0x26c00000, 0xffe00000, T_rSr },
  { "subf3",  0x26e00000, 0xffe00000, T_SSr },
  { "subf3",  0x36800000, 0xffe00000, T_Jrr }, /* C4x */
  { "subf3",  0x36a00000, 0xffe00000, T_Rrr }, /* C4x */
  { "subf3",  0x36c00000, 0xffe00000, T_JRr }, /* C4x */
  { "subf3",  0x36e00000, 0xffe00000, T_RRr }, /* C4x */
  { "subi",   0x18000000, 0xffe00000, G_r_r },
  { "subi",   0x18200000, 0xffe00000, G_T_r },
  { "subi",   0x18400000, 0xffe00000, G_Q_r },
  { "subi",   0x18600000, 0xffe00000, G_I_r },
  { "subi",   0x27000000, 0xffe00000, T_rrr },
  { "subi",   0x27200000, 0xffe00000, T_Srr },
  { "subi",   0x27400000, 0xffe00000, T_rSr },
  { "subi",   0x27600000, 0xffe00000, T_SSr },
  { "subi",   0x37000000, 0xffe00000, T_Jrr }, /* C4x */
  { "subi",   0x37200000, 0xffe00000, T_Rrr }, /* C4x */
  { "subi",   0x37400000, 0xffe00000, T_JRr }, /* C4x */
  { "subi",   0x37600000, 0xffe00000, T_RRr }, /* C4x */
  { "subi3",  0x27000000, 0xffe00000, T_rrr },
  { "subi3",  0x27200000, 0xffe00000, T_Srr },
  { "subi3",  0x27400000, 0xffe00000, T_rSr },
  { "subi3",  0x27600000, 0xffe00000, T_SSr },
  { "subi3",  0x37000000, 0xffe00000, T_Jrr }, /* C4x */
  { "subi3",  0x37200000, 0xffe00000, T_Rrr }, /* C4x */
  { "subi3",  0x37400000, 0xffe00000, T_JRr }, /* C4x */
  { "subi3",  0x37600000, 0xffe00000, T_RRr }, /* C4x */
  { "subrb",  0x18800000, 0xffe00000, G_r_r },
  { "subrb",  0x18a00000, 0xffe00000, G_T_r },
  { "subrb",  0x18c00000, 0xffe00000, G_Q_r },
  { "subrb",  0x18e00000, 0xffe00000, G_I_r },
  { "subrf",  0x19000000, 0xffe00000, G_r_r },
  { "subrf",  0x19200000, 0xffe00000, G_T_r },
  { "subrf",  0x19400000, 0xffe00000, G_Q_r },
  { "subrf",  0x19600000, 0xffe00000, G_F_r },
  { "subri",  0x19800000, 0xffe00000, G_r_r },
  { "subri",  0x19a00000, 0xffe00000, G_T_r },
  { "subri",  0x19c00000, 0xffe00000, G_Q_r },
  { "subri",  0x19e00000, 0xffe00000, G_I_r },
  { "swi",    0x66000000, 0xffffffff, "" },
  { "trapB",  0x74000000, 0xffe00000, "V" },
  { "trap",   0x74000000, 0xffe00000, "V" }, /* synonym for trapu */
  { "tstb",   0x1a000000, 0xffe00000, G_r_r },
  { "tstb",   0x1a200000, 0xffe00000, G_T_r },
  { "tstb",   0x1a400000, 0xffe00000, G_Q_r },
  { "tstb",   0x1a600000, 0xffe00000, G_L_r },
  { "tstb",   0x27800000, 0xffe00000, T_rr_ },
  { "tstb",   0x27a00000, 0xffe00000, T_rS_ },
  { "tstb",   0x27c00000, 0xffe00000, T_Sr_ },
  { "tstb",   0x27e00000, 0xffe00000, T_SS_ },
  { "tstb",   0x37800000, 0xffe00000, T_Jr_ }, /* C4x */
  { "tstb",   0x37800000, 0xffe00000, T_rJ_ }, /* C4x */
  { "tstb",   0x37a00000, 0xffe00000, T_rR_ }, /* C4x */
  { "tstb",   0x37a00000, 0xffe00000, T_Rr_ }, /* C4x */
  { "tstb",   0x37c00000, 0xffe00000, T_JR_ }, /* C4x */
  { "tstb",   0x37c00000, 0xffe00000, T_RJ_ }, /* C4x */
  { "tstb",   0x37e00000, 0xffe00000, T_RR_ }, /* C4x */
  { "tstb3",  0x27800000, 0xffe00000, T_rr_ },
  { "tstb3",  0x27a00000, 0xffe00000, T_rS_ },
  { "tstb3",  0x27c00000, 0xffe00000, T_Sr_ },
  { "tstb3",  0x27e00000, 0xffe00000, T_SS_ },
  { "tstb3",  0x37800000, 0xffe00000, T_Jr_ }, /* C4x */
  { "tstb3",  0x37800000, 0xffe00000, T_rJ_ }, /* C4x */
  { "tstb3",  0x37a00000, 0xffe00000, T_rR_ }, /* C4x */
  { "tstb3",  0x37a00000, 0xffe00000, T_Rr_ }, /* C4x */
  { "tstb3",  0x37c00000, 0xffe00000, T_JR_ }, /* C4x */
  { "tstb3",  0x37c00000, 0xffe00000, T_RJ_ }, /* C4x */
  { "tstb3",  0x37e00000, 0xffe00000, T_RR_ }, /* C4x */
  { "xor",    0x1a800000, 0xffe00000, G_r_r },
  { "xor",    0x1aa00000, 0xffe00000, G_T_r },
  { "xor",    0x1ac00000, 0xffe00000, G_Q_r },
  { "xor",    0x1ae00000, 0xffe00000, G_L_r },
  { "xor",    0x28000000, 0xffe00000, T_rrr },
  { "xor",    0x28200000, 0xffe00000, T_Srr },
  { "xor",    0x28400000, 0xffe00000, T_rSr },
  { "xor",    0x28600000, 0xffe00000, T_SSr },
  { "xor",    0x38000000, 0xffe00000, T_Jrr }, /* C4x */
  { "xor",    0x38000000, 0xffe00000, T_rJr }, /* C4x */
  { "xor",    0x38200000, 0xffe00000, T_rRr }, /* C4x */
  { "xor",    0x38200000, 0xffe00000, T_Rrr }, /* C4x */
  { "xor",    0x3c400000, 0xffe00000, T_JRr }, /* C4x */
  { "xor",    0x3c400000, 0xffe00000, T_RJr }, /* C4x */
  { "xor",    0x3c600000, 0xffe00000, T_RRr }, /* C4x */
  { "xor3",   0x28000000, 0xffe00000, T_rrr },
  { "xor3",   0x28200000, 0xffe00000, T_Srr },
  { "xor3",   0x28400000, 0xffe00000, T_rSr },
  { "xor3",   0x28600000, 0xffe00000, T_SSr },
  { "xor3",   0x38000000, 0xffe00000, T_Jrr }, /* C4x */
  { "xor3",   0x38000000, 0xffe00000, T_rJr }, /* C4x */
  { "xor3",   0x38200000, 0xffe00000, T_rRr }, /* C4x */
  { "xor3",   0x38200000, 0xffe00000, T_Rrr }, /* C4x */
  { "xor3",   0x3c400000, 0xffe00000, T_JRr }, /* C4x */
  { "xor3",   0x3c400000, 0xffe00000, T_RJr }, /* C4x */
  { "xor3",   0x3c600000, 0xffe00000, T_RRr }, /* C4x */
    
  /* Dummy entry, not included in c3x_num_insts.  This
     lets code examine entry i + 1 without checking
     if we've run off the end of the table.  */
  { "",      0x0, 0x00, "" }
};

const unsigned int c3x_num_insts = (((sizeof c3x_insts) / (sizeof c3x_insts[0])) - 1);

/* Define c4x additional opcodes for assembler and disassembler.  */
static const c4x_inst_t c4x_insts[] =
{
  /* Parallel instructions.  */
  { "frieee_stf",    0xf2000000, 0xfe000000, P_Sr_rS },
  { "toieee_stf",    0xf0000000, 0xfe000000, P_Sr_rS },

  { "bBaf",    0x68a00000, 0xffe00000, "Q" },
  { "bBaf",    0x6aa00000, 0xffe00000, "P" },
  { "baf",     0x68a00000, 0xffe00000, "Q" }, /* synonym for buaf */
  { "baf",     0x6aa00000, 0xffe00000, "P" }, /* synonym for buaf */
  { "bBat",    0x68600000, 0xffe00000, "Q" },
  { "bBat",    0x6a600000, 0xffe00000, "P" },
  { "bat",     0x68600000, 0xffe00000, "Q" }, /* synonym for buat */
  { "bat",     0x6a600000, 0xffe00000, "P" }, /* synonym for buat */
  { "laj",     0x63000000, 0xff000000, "B" },
  { "lajB",    0x70200000, 0xffe00000, "Q" },
  { "lajB",    0x72200000, 0xffe00000, "P" },
  { "latB",    0x74800000, 0xffe00000, "V" },

  { "frieee",  0x1c000000, 0xffe00000, G_r_r },
  { "frieee",  0x1c200000, 0xffe00000, G_T_r },
  { "frieee",  0x1c400000, 0xffe00000, G_Q_r },
  { "frieee",  0x1c600000, 0xffe00000, G_F_r },

  { "lb0",     0xb0000000, 0xffe00000, G_r_r },
  { "lb0",     0xb0200000, 0xffe00000, G_T_r },
  { "lb0",     0xb0400000, 0xffe00000, G_Q_r },
  { "lb0",     0xb0600000, 0xffe00000, G_I_r },
  { "lbu0",    0xb2000000, 0xffe00000, G_r_r },
  { "lbu0",    0xb2200000, 0xffe00000, G_T_r },
  { "lbu0",    0xb2400000, 0xffe00000, G_Q_r },
  { "lbu0",    0xb2600000, 0xffe00000, G_L_r },
  { "lb1",     0xb0800000, 0xffe00000, G_r_r },
  { "lb1",     0xb0a00000, 0xffe00000, G_T_r },
  { "lb1",     0xb0c00000, 0xffe00000, G_Q_r },
  { "lb1",     0xb0e00000, 0xffe00000, G_I_r },
  { "lbu1",    0xb2800000, 0xffe00000, G_r_r },
  { "lbu1",    0xb2a00000, 0xffe00000, G_T_r },
  { "lbu1",    0xb2c00000, 0xffe00000, G_Q_r },
  { "lbu1",    0xb2e00000, 0xffe00000, G_L_r },
  { "lb2",     0xb1000000, 0xffe00000, G_r_r },
  { "lb2",     0xb1200000, 0xffe00000, G_T_r },
  { "lb2",     0xb1400000, 0xffe00000, G_Q_r },
  { "lb2",     0xb1600000, 0xffe00000, G_I_r },
  { "lbu2",    0xb3000000, 0xffe00000, G_r_r },
  { "lbu2",    0xb3200000, 0xffe00000, G_T_r },
  { "lbu2",    0xb3400000, 0xffe00000, G_Q_r },
  { "lbu2",    0xb3600000, 0xffe00000, G_L_r },
  { "lb3",     0xb1800000, 0xffe00000, G_r_r },
  { "lb3",     0xb1a00000, 0xffe00000, G_T_r },
  { "lb3",     0xb1c00000, 0xffe00000, G_Q_r },
  { "lb3",     0xb1e00000, 0xffe00000, G_I_r },
  { "lbu3",    0xb3800000, 0xffe00000, G_r_r },
  { "lbu3",    0xb3a00000, 0xffe00000, G_T_r },
  { "lbu3",    0xb3c00000, 0xffe00000, G_Q_r },
  { "lbu3",    0xb3e00000, 0xffe00000, G_L_r },
  { "lda",     0x1e800000, 0xffe00000, "Q,Y" }, 
  { "lda",     0x1ea00000, 0xffe00000, "@,Y" }, 
  { "lda",     0x1ec00000, 0xffe00000, "*,Y" }, 
  { "lda",     0x1ee00000, 0xffe00000, "S,Y" }, 
  { "ldep",    0x76000000, 0xffe00000, "X,R" }, 
  { "ldhi",    0x1fe00000, 0xffe00000, G_L_r },
  { "ldhi",    0x1fe00000, 0xffe00000, "#,R" },
  { "ldpe",    0x76800000, 0xffe00000, "Q,Z" }, 
  { "ldpk",    0x1F700000, 0xffff0000, "#" },
  { "lh0",     0xba000000, 0xffe00000, G_r_r },
  { "lh0",     0xba200000, 0xffe00000, G_T_r },
  { "lh0",     0xba400000, 0xffe00000, G_Q_r },
  { "lh0",     0xba600000, 0xffe00000, G_I_r },
  { "lhu0",    0xbb000000, 0xffe00000, G_r_r },
  { "lhu0",    0xbb200000, 0xffe00000, G_T_r },
  { "lhu0",    0xbb400000, 0xffe00000, G_Q_r },
  { "lhu0",    0xbb600000, 0xffe00000, G_L_r },
  { "lh1",     0xba800000, 0xffe00000, G_r_r },
  { "lh1",     0xbaa00000, 0xffe00000, G_T_r },
  { "lh1",     0xbac00000, 0xffe00000, G_Q_r },
  { "lh1",     0xbae00000, 0xffe00000, G_I_r },
  { "lhu1",    0xbb800000, 0xffe00000, G_r_r },
  { "lhu1",    0xbba00000, 0xffe00000, G_T_r },
  { "lhu1",    0xbbc00000, 0xffe00000, G_Q_r },
  { "lhu1",    0xbbe00000, 0xffe00000, G_L_r },
  { "lwl0",    0xb4000000, 0xffe00000, G_r_r },
  { "lwl0",    0xb4200000, 0xffe00000, G_T_r },
  { "lwl0",    0xb4400000, 0xffe00000, G_Q_r },
  { "lwl0",    0xb4600000, 0xffe00000, G_I_r },
  { "lwl1",    0xb4800000, 0xffe00000, G_r_r },
  { "lwl1",    0xb4a00000, 0xffe00000, G_T_r },
  { "lwl1",    0xb4c00000, 0xffe00000, G_Q_r },
  { "lwl1",    0xb4e00000, 0xffe00000, G_I_r },
  { "lwl2",    0xb5000000, 0xffe00000, G_r_r },
  { "lwl2",    0xb5200000, 0xffe00000, G_T_r },
  { "lwl2",    0xb5400000, 0xffe00000, G_Q_r },
  { "lwl2",    0xb5600000, 0xffe00000, G_I_r },
  { "lwl3",    0xb5800000, 0xffe00000, G_r_r },
  { "lwl3",    0xb5a00000, 0xffe00000, G_T_r },
  { "lwl3",    0xb5c00000, 0xffe00000, G_Q_r },
  { "lwl3",    0xb5e00000, 0xffe00000, G_I_r },
  { "lwr0",    0xb6000000, 0xffe00000, G_r_r },
  { "lwr0",    0xb6200000, 0xffe00000, G_T_r },
  { "lwr0",    0xb6400000, 0xffe00000, G_Q_r },
  { "lwr0",    0xb6600000, 0xffe00000, G_I_r },
  { "lwr1",    0xb6800000, 0xffe00000, G_r_r },
  { "lwr1",    0xb6a00000, 0xffe00000, G_T_r },
  { "lwr1",    0xb6c00000, 0xffe00000, G_Q_r },
  { "lwr1",    0xb6e00000, 0xffe00000, G_I_r },
  { "lwr2",    0xb7000000, 0xffe00000, G_r_r },
  { "lwr2",    0xb7200000, 0xffe00000, G_T_r },
  { "lwr2",    0xb7400000, 0xffe00000, G_Q_r },
  { "lwr2",    0xb7600000, 0xffe00000, G_I_r },
  { "lwr3",    0xb7800000, 0xffe00000, G_r_r },
  { "lwr3",    0xb7a00000, 0xffe00000, G_T_r },
  { "lwr3",    0xb7c00000, 0xffe00000, G_Q_r },
  { "lwr3",    0xb7e00000, 0xffe00000, G_I_r },
  { "mb0",     0xb8000000, 0xffe00000, G_r_r },
  { "mb0",     0xb8200000, 0xffe00000, G_T_r },
  { "mb0",     0xb8400000, 0xffe00000, G_Q_r },
  { "mb0",     0xb8600000, 0xffe00000, G_I_r },
  { "mb1",     0xb8800000, 0xffe00000, G_r_r },
  { "mb1",     0xb8a00000, 0xffe00000, G_T_r },
  { "mb1",     0xb8c00000, 0xffe00000, G_Q_r },
  { "mb1",     0xb8e00000, 0xffe00000, G_I_r },
  { "mb2",     0xb9000000, 0xffe00000, G_r_r },
  { "mb2",     0xb9200000, 0xffe00000, G_T_r },
  { "mb2",     0xb9400000, 0xffe00000, G_Q_r },
  { "mb2",     0xb9600000, 0xffe00000, G_I_r },
  { "mb3",     0xb9800000, 0xffe00000, G_r_r },
  { "mb3",     0xb9a00000, 0xffe00000, G_T_r },
  { "mb3",     0xb9c00000, 0xffe00000, G_Q_r },
  { "mb3",     0xb9e00000, 0xffe00000, G_I_r },
  { "mh0",     0xbc000000, 0xffe00000, G_r_r },
  { "mh0",     0xbc200000, 0xffe00000, G_T_r },
  { "mh0",     0xbc400000, 0xffe00000, G_Q_r },
  { "mh0",     0xbc600000, 0xffe00000, G_I_r },
  { "mh1",     0xbc800000, 0xffe00000, G_r_r },
  { "mh1",     0xbca00000, 0xffe00000, G_T_r },
  { "mh1",     0xbcc00000, 0xffe00000, G_Q_r },
  { "mh1",     0xbce00000, 0xffe00000, G_I_r },
  { "mh2",     0xbd000000, 0xffe00000, G_r_r },
  { "mh2",     0xbd200000, 0xffe00000, G_T_r },
  { "mh2",     0xbd400000, 0xffe00000, G_Q_r },
  { "mh2",     0xbd600000, 0xffe00000, G_I_r },
  { "mh3",     0xbd800000, 0xffe00000, G_r_r },
  { "mh3",     0xbda00000, 0xffe00000, G_T_r },
  { "mh3",     0xbdc00000, 0xffe00000, G_Q_r },
  { "mh3",     0xbde00000, 0xffe00000, G_I_r },
  { "mpyshi",  0x1d800000, 0xffe00000, G_r_r },
  { "mpyshi",  0x1da00000, 0xffe00000, G_T_r },
  { "mpyshi",  0x1dc00000, 0xffe00000, G_Q_r },
  { "mpyshi",  0x1de00000, 0xffe00000, G_I_r },
  { "mpyshi",  0x28800000, 0xffe00000, T_rrr },
  { "mpyshi",  0x28a00000, 0xffe00000, T_Srr },
  { "mpyshi",  0x28c00000, 0xffe00000, T_rSr },
  { "mpyshi",  0x28e00000, 0xffe00000, T_SSr },
  { "mpyshi",  0x38800000, 0xffe00000, T_Jrr }, /* C4x */
  { "mpyshi",  0x38800000, 0xffe00000, T_rJr }, /* C4x */
  { "mpyshi",  0x38a00000, 0xffe00000, T_rRr }, /* C4x */
  { "mpyshi",  0x38a00000, 0xffe00000, T_Rrr }, /* C4x */
  { "mpyshi",  0x38c00000, 0xffe00000, T_JRr }, /* C4x */
  { "mpyshi",  0x38c00000, 0xffe00000, T_RJr }, /* C4x */
  { "mpyshi",  0x38e00000, 0xffe00000, T_RRr }, /* C4x */
  { "mpyshi3", 0x28800000, 0xffe00000, T_rrr },
  { "mpyshi3", 0x28a00000, 0xffe00000, T_Srr },
  { "mpyshi3", 0x28c00000, 0xffe00000, T_rSr },
  { "mpyshi3", 0x28e00000, 0xffe00000, T_SSr },
  { "mpyshi3", 0x38800000, 0xffe00000, T_Jrr }, /* C4x */
  { "mpyshi3", 0x38800000, 0xffe00000, T_rJr }, /* C4x */
  { "mpyshi3", 0x38a00000, 0xffe00000, T_rRr }, /* C4x */
  { "mpyshi3", 0x38a00000, 0xffe00000, T_Rrr }, /* C4x */
  { "mpyshi3", 0x38c00000, 0xffe00000, T_JRr }, /* C4x */
  { "mpyshi3", 0x38c00000, 0xffe00000, T_RJr }, /* C4x */
  { "mpyshi3", 0x38e00000, 0xffe00000, T_RRr }, /* C4x */
  { "mpyuhi",  0x1e000000, 0xffe00000, G_r_r },
  { "mpyuhi",  0x1e200000, 0xffe00000, G_T_r },
  { "mpyuhi",  0x1e400000, 0xffe00000, G_Q_r },
  { "mpyuhi",  0x1e600000, 0xffe00000, G_I_r },
  { "mpyuhi",  0x29000000, 0xffe00000, T_rrr },
  { "mpyuhi",  0x29200000, 0xffe00000, T_Srr },
  { "mpyuhi",  0x29400000, 0xffe00000, T_rSr },
  { "mpyuhi",  0x29600000, 0xffe00000, T_SSr },
  { "mpyuhi",  0x39000000, 0xffe00000, T_Jrr }, /* C4x */
  { "mpyuhi",  0x39000000, 0xffe00000, T_rJr }, /* C4x */
  { "mpyuhi",  0x39200000, 0xffe00000, T_rRr }, /* C4x */
  { "mpyuhi",  0x39200000, 0xffe00000, T_Rrr }, /* C4x */
  { "mpyuhi",  0x39400000, 0xffe00000, T_JRr }, /* C4x */
  { "mpyuhi",  0x39400000, 0xffe00000, T_RJr }, /* C4x */
  { "mpyuhi",  0x39600000, 0xffe00000, T_RRr }, /* C4x */
  { "mpyuhi3", 0x29000000, 0xffe00000, T_rrr },
  { "mpyuhi3", 0x29200000, 0xffe00000, T_Srr },
  { "mpyuhi3", 0x29400000, 0xffe00000, T_rSr },
  { "mpyuhi3", 0x29600000, 0xffe00000, T_SSr },
  { "mpyuhi3", 0x39000000, 0xffe00000, T_Jrr }, /* C4x */
  { "mpyuhi3", 0x39000000, 0xffe00000, T_rJr }, /* C4x */
  { "mpyuhi3", 0x39200000, 0xffe00000, T_rRr }, /* C4x */
  { "mpyuhi3", 0x39200000, 0xffe00000, T_Rrr }, /* C4x */
  { "mpyuhi3", 0x39400000, 0xffe00000, T_JRr }, /* C4x */
  { "mpyuhi3", 0x39400000, 0xffe00000, T_RJr }, /* C4x */
  { "mpyuhi3", 0x39600000, 0xffe00000, T_RRr }, /* C4x */
  { "rcpf",    0x1d000000, 0xffe00000, G_r_r },
  { "rcpf",    0x1d200000, 0xffe00000, G_T_r },
  { "rcpf",    0x1d400000, 0xffe00000, G_Q_r },
  { "rcpf",    0x1d600000, 0xffe00000, G_F_r },
  { "retiBd",  0x78200000, 0xffe00000, "" },
  { "retid",   0x78200000, 0xffe00000, "" }, /* synonym for retiud */
  { "rptbd",   0x79800000, 0xff000000, "Q" },  
  { "rptbd",   0x65000000, 0xff000000, "B" },  
  { "rsqrf",   0x1c800000, 0xffe00000, G_r_r },
  { "rsqrf",   0x1ca00000, 0xffe00000, G_T_r },
  { "rsqrf",   0x1cc00000, 0xffe00000, G_Q_r },
  { "rsqrf",   0x1ce00000, 0xffe00000, G_F_r },
  { "stik",    0x15000000, 0xffe00000, "T,@" },
  { "stik",    0x15600000, 0xffe00000, "T,*" },
  { "toieee",  0x1b800000, 0xffe00000, G_r_r },
  { "toieee",  0x1ba00000, 0xffe00000, G_T_r },
  { "toieee",  0x1bc00000, 0xffe00000, G_Q_r },
  { "toieee",  0x1be00000, 0xffe00000, G_F_r },
  { "idle2",   0x06000001, 0xffffffff, "" }, 
    
  /* Dummy entry, not included in num_insts.  This
     lets code examine entry i+1 without checking
     if we've run off the end of the table.  */
  { "",      0x0, 0x00, "" }
};

const unsigned int c4x_num_insts = (((sizeof c4x_insts) / (sizeof c4x_insts[0])) - 1);
    

struct c4x_cond
{
  char *        name;
  unsigned long cond;
};

typedef struct c4x_cond c4x_cond_t;

/* Define conditional branch/load suffixes.  Put desired form for
   disassembler last.  */
static const c4x_cond_t c4x_conds[] =
{
  { "u",    0x00 },
  { "c",    0x01 }, { "lo",  0x01 },
  { "ls",   0x02 },
  { "hi",   0x03 },
  { "nc",   0x04 }, { "hs",  0x04 },
  { "z",    0x05 }, { "eq",  0x05 },
  { "nz",   0x06 }, { "ne",  0x06 },
  { "n",    0x07 }, { "l",   0x07 }, { "lt",  0x07 },
  { "le",   0x08 },
  { "p",    0x09 }, { "gt",  0x09 },
  { "nn",   0x0a }, { "ge",  0x0a },
  { "nv",   0x0c },
  { "v",    0x0d },
  { "nuf",  0x0e },
  { "uf",   0x0f },
  { "nlv",  0x10 },
  { "lv",   0x11 },
  { "nluf", 0x12 },
  { "luf",  0x13 },
  { "zuf",  0x14 },
  /* Dummy entry, not included in num_conds.  This
     lets code examine entry i+1 without checking
     if we've run off the end of the table.  */
  { "",      0x0}
};

const unsigned int num_conds = (((sizeof c4x_conds) / (sizeof c4x_conds[0])) - 1);

struct c4x_indirect
{
  char *        name;
  unsigned long modn;
};

typedef struct c4x_indirect c4x_indirect_t;

/* Define indirect addressing modes where:
   d displacement (signed)
   y ir0
   z ir1  */

static const c4x_indirect_t c4x_indirects[] =
{
  { "*+a(d)",   0x00 },
  { "*-a(d)",   0x01 },
  { "*++a(d)",  0x02 },
  { "*--a(d)",  0x03 },
  { "*a++(d)",  0x04 },
  { "*a--(d)",  0x05 },
  { "*a++(d)%", 0x06 },
  { "*a--(d)%", 0x07 },
  { "*+a(y)",   0x08 },
  { "*-a(y)",   0x09 },
  { "*++a(y)",  0x0a },
  { "*--a(y)",  0x0b },
  { "*a++(y)",  0x0c },
  { "*a--(y)",  0x0d },
  { "*a++(y)%", 0x0e },
  { "*a--(y)%", 0x0f },
  { "*+a(z)",   0x10 },
  { "*-a(z)",   0x11 },
  { "*++a(z)",  0x12 },
  { "*--a(z)",  0x13 },
  { "*a++(z)",  0x14 },
  { "*a--(z)",  0x15 },
  { "*a++(z)%", 0x16 },
  { "*a--(z)%", 0x17 },
  { "*a",       0x18 },
  { "*a++(y)b", 0x19 },
  /* Dummy entry, not included in num_indirects.  This
     lets code examine entry i+1 without checking
     if we've run off the end of the table.  */
  { "",      0x0}
};

#define C3X_MODN_MAX 0x19

const unsigned int num_indirects = (((sizeof c4x_indirects) / (sizeof c4x_indirects[0])) - 1);
