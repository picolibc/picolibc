/* HPPA ELF support for BFD.
   Copyright (C) 1993, 1994 Free Software Foundation, Inc.

This file is part of BFD, the Binary File Descriptor library.

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

/* This file holds definitions specific to the HPPA ELF ABI.  Note
   that most of this is not actually implemented by BFD.  */

#ifndef _ELF_HPPA_H
#define _ELF_HPPA_H

/* Processor specific flags for the ELF header e_flags field.  */

/* Target processor IDs to be placed in the low 16 bits of the flags
   field.  Note these names are shared with SOM, and therefore do not
   follow ELF naming conventions.  */

/* PA 1.0 big endian.  */
#ifndef CPU_PA_RISC1_0
#define CPU_PA_RISC1_0		0x0000020b
#endif

/* PA 1.1 big endian.  */
#ifndef CPU_PA_RISC1_1
#define CPU_PA_RISC1_1		0x00000210
#endif

/* PA 1.0 little endian (unsupported) is 0x0000028b.  */
/* PA 1.1 little endian (unsupported) is 0x00000290.  */

/* Trap null address dereferences.  */
#define ELF_PARISC_TRAPNIL	0x00010000

/* .PARISC.archext section is present.  */
#define EF_PARISC_EXT		0x00020000

/* Processor specific section types.  */

/* Holds the global offset table, a table of pointers to external
   data.  */
#define SHT_PARISC_GOT		SHT_LOPROC+0

/* Nonloadable section containing information in architecture
   extensions used by the code.  */
#define SHT_PARISC_ARCH		SHT_LOPROC+1

/* Section in which $global$ is defined.  */
#define SHT_PARISC_GLOBAL	SHT_LOPROC+2

/* Section holding millicode routines (mul, div, rem, dyncall, etc.  */
#define SHT_PARISC_MILLI	SHT_LOPROC+3

/* Section holding unwind information for use by debuggers.  */
#define SHT_PARISC_UNWIND	SHT_LOPROC+4

/* Section holding the procedure linkage table.  */
#define SHT_PARISC_PLT		SHT_LOPROC+5

/* Short initialized and uninitialized data.  */
#define SHT_PARISC_SDATA	SHT_LOPROC+6
#define SHT_PARISC_SBSS		SHT_LOPROC+7

/* Optional section holding argument location/relocation info.  */
#define SHT_PARISC_SYMEXTN	SHT_LOPROC+8

/* Option section for linker stubs.  */
#define SHT_PARISC_STUBS	SHT_LOPROC+9

/* Processor specific section flags.  */

/* This section is near the global data pointer and thus allows short
   addressing modes to be used.  */
#define SHF_PARISC_SHORT        0x20000000

/* Processor specific symbol types.  */

/* Millicode function entry point.  */
#define STT_PARISC_MILLICODE	STT_LOPROC+0


/* ELF/HPPA relocation types */

#include "reloc-macros.h"

START_RELOC_NUMBERS (elf32_hppa_reloc_type)
     RELOC_NUMBER (R_PARISC_NONE,      0)	/* No reloc */
     
     /* These relocation types do simple base + offset relocations.  */

     RELOC_NUMBER (R_PARISC_DIR32,  0x01)
     RELOC_NUMBER (R_PARISC_DIR21L, 0x02)
     RELOC_NUMBER (R_PARISC_DIR17R, 0x03)
     RELOC_NUMBER (R_PARISC_DIR17F, 0x04)
     RELOC_NUMBER (R_PARISC_DIR14R, 0x06)

    /* PC-relative relocation types
       Typically used for calls.
       Note PCREL17C and PCREL17F differ only in overflow handling.
       PCREL17C never reports a relocation error.

       When supporting argument relocations, function calls must be
       accompanied by parameter relocation information.  This information is 
       carried in the ten high-order bits of the addend field.  The remaining
       22 bits of of the addend field are sign-extended to form the Addend.

       Note the code to build argument relocations depends on the 
       addend being zero.  A consequence of this limitation is GAS
       can not perform relocation reductions for function symbols.  */
     
     RELOC_NUMBER (R_PARISC_PCREL21L, 0x0a)
     RELOC_NUMBER (R_PARISC_PCREL17R, 0x0b)
     RELOC_NUMBER (R_PARISC_PCREL17F, 0x0c)
     RELOC_NUMBER (R_PARISC_PCREL17C, 0x0d)
     RELOC_NUMBER (R_PARISC_PCREL14R, 0x0e)
     RELOC_NUMBER (R_PARISC_PCREL14F, 0x0f)

    /* DP-relative relocation types.  */
     RELOC_NUMBER (R_PARISC_DPREL21L, 0x12)
     RELOC_NUMBER (R_PARISC_DPREL14R, 0x16)
     RELOC_NUMBER (R_PARISC_DPREL14F, 0x17)

    /* Data linkage table (DLT) relocation types

       SOM DLT_REL fixup requests are used to for static data references
       from position-independent code within shared libraries.  They are
       similar to the GOT relocation types in some SVR4 implementations.  */

     RELOC_NUMBER (R_PARISC_DLTREL21L, 0x1a)
     RELOC_NUMBER (R_PARISC_DLTREL14R, 0x1e)
     RELOC_NUMBER (R_PARISC_DLTREL14F, 0x1f)

    /* DLT indirect relocation types  */
     RELOC_NUMBER (R_PARISC_DLTIND21L, 0x22)
     RELOC_NUMBER (R_PARISC_DLTIND14R, 0x26)
     RELOC_NUMBER (R_PARISC_DLTIND14F, 0x27)

    /* Base relative relocation types.  Ugh.  These imply lots of state */
     RELOC_NUMBER (R_PARISC_SETBASE,    0x28)
     RELOC_NUMBER (R_PARISC_BASEREL32,  0x29)
     RELOC_NUMBER (R_PARISC_BASEREL21L, 0x2a)
     RELOC_NUMBER (R_PARISC_BASEREL17R, 0x2b)
     RELOC_NUMBER (R_PARISC_BASEREL17F, 0x2c)
     RELOC_NUMBER (R_PARISC_BASEREL14R, 0x2e)
     RELOC_NUMBER (R_PARISC_BASEREL14F, 0x2f)

    /* Segment relative relocation types.  */
     RELOC_NUMBER (R_PARISC_TEXTREL32, 0x31)
     RELOC_NUMBER (R_PARISC_DATAREL32, 0x39)

    /* Plabel relocation types.  */
     RELOC_NUMBER (R_PARISC_PLABEL32,  0x41)
     RELOC_NUMBER (R_PARISC_PLABEL21L, 0x42)
     RELOC_NUMBER (R_PARISC_PLABEL14R, 0x46)

    /* PLT relocations.  */
     RELOC_NUMBER (R_PARISC_PLTIND21L, 0x82)
     RELOC_NUMBER (R_PARISC_PLTIND14R, 0x86)
     RELOC_NUMBER (R_PARISC_PLTIND14F, 0x87)

    /* Misc relocation types.  */
     RELOC_NUMBER (R_PARISC_COPY,     0x88)
     RELOC_NUMBER (R_PARISC_GLOB_DAT, 0x89)
     RELOC_NUMBER (R_PARISC_JMP_SLOT, 0x8a)
     RELOC_NUMBER (R_PARISC_RELATIVE, 0x8b)
     
     EMPTY_RELOC (R_PARISC_UNIMPLEMENTED)
END_RELOC_NUMBERS

#ifndef RELOC_MACROS_GEN_FUNC
typedef enum elf32_hppa_reloc_type elf32_hppa_reloc_type;
#endif

#endif /* _ELF_HPPA_H */
