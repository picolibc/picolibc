/* crx.h -- Header file for CRX opcode and register tables.
   Copyright 2004 Free Software Foundation, Inc.
   Contributed by Tomer Levi, NSC, Israel.
   Originally written for GAS 2.12 by Tomer Levi, NSC, Israel.
   Updates, BFDizing, GNUifying and ELF support by Tomer Levi.

   This file is part of GAS, GDB and the GNU binutils.

   GAS, GDB, and GNU binutils is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at your
   option) any later version.

   GAS, GDB, and GNU binutils are distributed in the hope that they will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#ifndef _CRX_H_
#define _CRX_H_

/* CRX core/debug Registers :
   The enums are used as indices to CRX registers table (crx_regtab).
   Therefore, order MUST be preserved.  */

typedef enum
  {
    /* 32-bit general purpose registers.  */
    r0, r1, r2, r3, r4, r5, r6, r7, r8, r9,
    r10, r11, r12, r13, r14, r15, ra, sp,
    /* 32-bit user registers.  */
    u0, u1, u2, u3, u4, u5, u6, u7, u8, u9,
    u10, u11, u12, u13, u14, u15, ura, usp,
    /* hi and lo registers.  */
    hi, lo,
    /* hi and lo user registers.  */
    uhi, ulo,
    /* Processor Status Register.  */
    psr,
    /* Interrupt Base Register.  */
    intbase,
    /* Interrupt Stack Pointer Register.  */
    isp,
    /* Configuration Register.  */
    cfg,
    /* Coprocessor Configuration Register.  */
    cpcfg,
    /* Coprocessor Enable Register.  */
    cen,
    /* Not a register.  */
    nullregister,
    MAX_REG
  }
reg;

/* CRX Coprocessor registers and special registers :
   The enums are used as indices to CRX coprocessor registers table
   (crx_copregtab). Therefore, order MUST be preserved.  */

typedef enum
  {
    /* Coprocessor registers.  */
    c0 = MAX_REG, c1, c2, c3, c4, c5, c6, c7, c8,
    c9, c10, c11, c12, c13, c14, c15,
    /* Coprocessor special registers.  */
    cs0, cs1 ,cs2, cs3, cs4, cs5, cs6, cs7, cs8,
    cs9, cs10, cs11, cs12, cs13, cs14, cs15,
    /* Not a Coprocessor register.  */
    nullcopregister,
    MAX_COPREG
  }
copreg;

/* CRX Register types. */

typedef enum
  {
    CRX_R_REGTYPE,    /*  r<N>	  */
    CRX_U_REGTYPE,    /*  u<N>	  */
    CRX_C_REGTYPE,    /*  c<N>	  */
    CRX_CS_REGTYPE,   /*  cs<N>	  */
    CRX_CFG_REGTYPE   /*  configuration register   */
  }
reg_type;

/* CRX argument types :
   The argument types correspond to instructions operands

   Argument types :
   r - register
   c - constant
   d - displacement
   ic - immediate
   icr - index register
   rbase - register base
   s - star ('*')
   copr - coprocessor register
   copsr - coprocessor special register.  */

typedef enum
  {
    arg_r, arg_c, arg_cr, arg_dc, arg_dcr, arg_sc,
    arg_ic, arg_icr, arg_rbase, arg_copr, arg_copsr,
    /* Not an argument.  */
    nullargs
  }
argtype;

/* CRX operand types :
   The operand types correspond to instructions operands.  */

typedef enum
  {
    dummy,
    /* 4-bit encoded constant.  */
    cst4,
    /* N-bit immediate.  */
    i16, i32,
    /* N-bit unsigned immediate.  */
    ui3, ui4, ui5, ui16,
    /* N-bit signed displacement.  */
    disps9, disps17, disps25, disps32,
    /* N-bit unsigned displacement.  */
    dispu5, 
    /* N-bit escaped displacement.  */
    dispe9,
    /* N-bit absolute address.  */
    abs16, abs32,
    /* Register relative.  */
    rbase, rbase_cst4,
    rbase_disps12, rbase_disps16, rbase_disps28, rbase_disps32,
    /* Register index.  */
    rindex_disps6, rindex_disps22,
    /* 4-bit genaral-purpose register specifier.  */
    regr, 
    /* 8-bit register address space.  */
    regr8,
    /* coprocessor register.  */
    copregr, 
    /* coprocessor special register.  */
    copsregr,
    /* Not an operand.  */
    nulloperand,
    /* Maximum supported operand.  */
    MAX_OPRD
  }
operand_type;

/* CRX instruction types.  */

#define NO_TYPE_INS       0
#define ARITH_INS         1
#define LD_STOR_INS       2
#define BRANCH_INS        3
#define ARITH_BYTE_INS    4
#define CMPBR_INS         5
#define SHIFT_INS         6
#define BRANCH_NEQ_INS    7
#define LD_STOR_INS_INC   8
#define STOR_IMM_INS	  9
#define CSTBIT_INS       10
#define SYS_INS		 11
#define JMP_INS		 12
#define MUL_INS		 13
#define DIV_INS		 14
#define COP_BRANCH_INS   15
#define COP_REG_INS      16
#define COPS_REG_INS     17
#define DCR_BRANCH_INS   18
#define MMC_INS          19
#define MMU_INS          20

/* Maximum value supported for instruction types.  */
#define CRX_INS_MAX	(1 << 5)
/* Mask to record an instruction type.  */
#define CRX_INS_MASK	(CRX_INS_MAX - 1)
/* Return instruction type, given instruction's attributes.  */
#define CRX_INS_TYPE(attr) ((attr) & CRX_INS_MASK)

/* Indicates whether this instruction has a register list as parameter.  */
#define REG_LIST	CRX_INS_MAX
/* The operands in binary and assembly are placed in reverse order.
   load - (REVERSE_MATCH)/store - (! REVERSE_MATCH).  */
#define REVERSE_MATCH  (1 << 6)

/* Kind of displacement map used DISPU[BWD]4.  */
#define DISPUB4	       (1 << 7)
#define DISPUW4	       (1 << 8)
#define DISPUD4	       (1 << 9)
#define DISPU4MAP      (DISPUB4 | DISPUW4 | DISPUD4)

/* Printing formats, where the instruction prefix isn't consecutive.  */
#define FMT_1	       (1 << 10)   /* 0xF0F00000 */
#define FMT_2	       (1 << 11)   /* 0xFFF0FF00 */
#define FMT_3	       (1 << 12)   /* 0xFFF00F00 */
#define FMT_4	       (1 << 13)   /* 0xFFF0F000 */
#define FMT_5	       (1 << 14)   /* 0xFFF0FFF0 */
#define FMT_CRX	       (FMT_1 | FMT_2 | FMT_3 | FMT_4 | FMT_5)

/* Indicates whether this instruction can be relaxed.  */
#define RELAXABLE      (1 << 15)

/* Indicates that instruction uses user registers (and not 
   general-purpose registers) as operands.  */
#define USER_REG       (1 << 16)

/* Indicates that instruction can perfom a cst4 mapping.  */
#define CST4MAP	       (1 << 17)

/* Instruction shouldn't allow 'sp' usage.  */
#define NO_SP	       (1 << 18)

/* Instruction shouldn't allow to push a register which is used as a rptr.  */
#define NO_RPTR	       (1 << 19)

/* Maximum operands per instruction.  */
#define MAX_OPERANDS	  5
/* Maximum words per instruction.  */
#define MAX_WORDS	  3
/* Maximum register name length. */
#define MAX_REGNAME_LEN	  10
/* Maximum instruction length. */
#define MAX_INST_LEN	  256


/* Values defined for the flags field of a struct operand_entry.  */

/* Operand must be an unsigned number.  */
#define OPERAND_UNSIGNED  (1 << 0)
/* Operand must be a signed number.  */
#define OPERAND_SIGNED	  (1 << 1)
/* A cst4 operand.  */
#define OPERAND_CST4	  (1 << 2)
/* Operand must be an even number.  */
#define OPERAND_EVEN	  (1 << 3)
/* Operand is shifted right.  */
#define OPERAND_SHIFT	  (1 << 4)
/* Operand is shifted right and decremented.  */
#define OPERAND_SHIFT_DEC (1 << 5)
/* Operand has reserved escape sequences.  */
#define OPERAND_ESC	  (1 << 6)

/* Single operand description.  */

typedef struct
  {
    /* Operand type.  */
    operand_type op_type;
    /* Operand location within the opcode.  */
    unsigned int shift;
  }
operand_desc;

/* Instruction data structure used in instruction table.  */

typedef struct
  {
    /* Name.  */
    const char *mnemonic;
    /* Size (in words).  */
    unsigned int size;
    /* Constant prefix (matched by the disassembler).  */
    unsigned long match;
    /* Match size (in bits).  */
    int match_bits;
    /* Attributes.  */
    unsigned int flags;
    /* Operands (always last, so unreferenced operands are initialized).  */
    operand_desc operands[MAX_OPERANDS];
  }
inst;

/* Data structure for a single instruction's arguments (Operands).  */

typedef struct
  {
    /* Register or base register.  */
    reg r;
    /* Index register.  */
    reg i_r;
    /* Coprocessor register.  */
    copreg cr;
    /* Constant/immediate/absolute value.  */
    unsigned long int constant;
    /* Scaled index mode.  */
    unsigned int scale;
    /* Argument type.  */
    argtype type;
    /* Size of the argument (in bits) required to represent.  */
    int size;
    /* Indicates whether a constant is positive or negative.  */
    int signflag;
  }
argument;

/* Internal structure to hold the various entities
   corresponding to the current assembling instruction.  */

typedef struct
  {
    /* Number of arguments.  */
    int nargs;
    /* The argument data structure for storing args (operands).  */
    argument arg[MAX_OPERANDS];
/* The following fields are required only by CRX-assembler.  */
#ifdef TC_CRX
    /* Expression used for setting the fixups (if any).  */
    expressionS exp;
    bfd_reloc_code_real_type rtype;
#endif /* TC_CRX */
    /* Instruction size (in bytes).  */
    int size;
  }
ins;

/* Structure to hold information about predefined operands.  */

typedef struct
  {
    /* Size (in bits).  */
    unsigned int bit_size;
    /* Argument type.  */
    argtype arg_type;
    /* One bit syntax flags.  */
    int flags;
  }
operand_entry;

/* Structure to hold trap handler information.  */

typedef struct
  {
    /* Trap name.  */
    char *name;
    /* Index in dispatch table.  */
    unsigned int entry;
  }
trap_entry;

/* Structure to hold information about predefined registers.  */

typedef struct
  {
    /* Name (string representation).  */
    char *name;
    /* Value (enum representation).  */
    union
    {
      /* Register.  */
      reg reg_val;
      /* Coprocessor register.  */
      copreg copreg_val;
    } value;
    /* Register image.  */
    int image;
    /* Register type.  */
    reg_type type;
  }
reg_entry;

/* Structure to hold a cst4 operand mapping.  */

typedef struct
  {
    /* The binary value which is written to the object file.  */
    int binary;
    /* The value which is mapped.  */
    int value;
  }
cst4_entry;

/* CRX opcode table.  */
extern const inst crx_instruction[];
extern const int crx_num_opcodes;
#define NUMOPCODES crx_num_opcodes

/* CRX operands table.  */
extern const operand_entry crx_optab[];

/* CRX registers table.  */
extern const reg_entry crx_regtab[];
extern const int crx_num_regs;
#define NUMREGS crx_num_regs

/* CRX coprocessor registers table.  */
extern const reg_entry crx_copregtab[];
extern const int crx_num_copregs;
#define NUMCOPREGS crx_num_copregs

/* CRX trap/interrupt table.  */
extern const trap_entry crx_traps[];
extern const int crx_num_traps;
#define NUMTRAPS crx_num_traps

/* cst4 operand mapping.  */
extern const cst4_entry cst4_map[];
extern const int cst4_maps;

/* Current instruction we're assembling.  */
extern const inst *instruction;

/* A macro for representing the instruction "constant" opcode, that is,
   the FIXED part of the instruction. The "constant" opcode is represented
   as a 32-bit unsigned long, where OPC is expanded (by a left SHIFT)
   over that range.  */
#define BIN(OPC,SHIFT)	(OPC << SHIFT)

/* Is the current instruction type is TYPE ?  */
#define IS_INSN_TYPE(TYPE)	      \
  (CRX_INS_TYPE(instruction->flags) == TYPE)

/* Is the current instruction mnemonic is MNEMONIC ?  */
#define IS_INSN_MNEMONIC(MNEMONIC)    \
  (strcmp(instruction->mnemonic,MNEMONIC) == 0)

/* Does the current instruction has register list ?  */
#define INST_HAS_REG_LIST	      \
  (instruction->flags & REG_LIST)

/* Long long type handling.  */
/* Replace all appearances of 'long long int' with LONGLONG.  */
typedef long long int LONGLONG;
typedef unsigned long long ULONGLONG;

#endif /* _CRX_H_ */
