/* ARC ELF support for BFD.
   Copyright 1995, 1997, 1998, 2000, 2001, 2002, 2005, 2006, 2007, 2008, 2009
   Free Software Foundation, Inc.
   Contributed by Doug Evans (dje@cygnus.com).

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
Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.  */

/* This file holds definitions specific to the ARC ELF ABI.  */

#ifndef _ELF_ARC_H
#define _ELF_ARC_H

#include "elf/reloc-macros.h"

/* Relocations.  */

START_RELOC_NUMBERS (elf_arc_reloc_type)
    RELOC_NUMBER (R_ARC_NONE, 0x0)
    RELOC_NUMBER (R_ARC_8, 0x1)
    RELOC_NUMBER (R_ARC_16,0x2)
    RELOC_NUMBER (R_ARC_24,0x3)
    RELOC_NUMBER (R_ARC_32,0x4)
    RELOC_NUMBER (R_ARC_B26,0x5)
    RELOC_NUMBER (R_ARC_B22_PCREL, 0x6)
    
    RELOC_NUMBER (R_ARC_H30,0x7)
    RELOC_NUMBER (R_ARC_N8, 0x8)
    RELOC_NUMBER (R_ARC_N16,0x9)
    RELOC_NUMBER (R_ARC_N24,0xA)
    RELOC_NUMBER (R_ARC_N32,0xB)
    RELOC_NUMBER (R_ARC_SDA,0xC)
    RELOC_NUMBER (R_ARC_SECTOFF,0xD)
	 
    RELOC_NUMBER (R_ARC_S21H_PCREL, 0xE)
    RELOC_NUMBER (R_ARC_S21W_PCREL, 0xF)
    RELOC_NUMBER (R_ARC_S25H_PCREL, 0x10)
    RELOC_NUMBER (R_ARC_S25W_PCREL, 0x11)

    RELOC_NUMBER (R_ARC_SDA32, 0x12)
    RELOC_NUMBER (R_ARC_SDA_LDST, 0x13)
    RELOC_NUMBER (R_ARC_SDA_LDST1, 0x14)
    RELOC_NUMBER (R_ARC_SDA_LDST2, 0x15)
    RELOC_NUMBER (R_ARC_SDA16_LD,0x16)
    RELOC_NUMBER (R_ARC_SDA16_LD1,0x17)
    RELOC_NUMBER (R_ARC_SDA16_LD2,0x18)
   

    RELOC_NUMBER (R_ARC_S13_PCREL,0x19 )

    RELOC_NUMBER (R_ARC_W, 0x1A)
    RELOC_NUMBER (R_ARC_32_ME, 0x1B)

    RELOC_NUMBER (R_ARC_N32_ME , 0x1C)
    RELOC_NUMBER (R_ARC_SECTOFF_ME, 0x1D)
    RELOC_NUMBER (R_ARC_SDA32_ME , 0x1E)
    RELOC_NUMBER (R_ARC_W_ME, 0x1F)
    RELOC_NUMBER (R_ARC_H30_ME, 0x20)

    RELOC_NUMBER (R_ARC_SECTOFF_U8, 0x21)
    RELOC_NUMBER (R_ARC_SECTOFF_S9, 0x22)

    
    RELOC_NUMBER (R_AC_SECTOFF_U8,   0x23)
    RELOC_NUMBER (R_AC_SECTOFF_U8_1, 0x24)
    RELOC_NUMBER (R_AC_SECTOFF_U8_2, 0x25)
   

    RELOC_NUMBER (R_AC_SECTOFF_S9,   0x26)
    RELOC_NUMBER (R_AC_SECTOFF_S9_1, 0x27)
    RELOC_NUMBER (R_AC_SECTOFF_S9_2, 0x28)
   

    RELOC_NUMBER (R_ARC_SECTOFF_ME_1 ,0x29)
    RELOC_NUMBER (R_ARC_SECTOFF_ME_2, 0x2A)
    RELOC_NUMBER (R_ARC_SECTOFF_1,    0x2B)
    RELOC_NUMBER (R_ARC_SECTOFF_2,    0x2C)


    RELOC_NUMBER (R_ARC_PC32, 0x32)		 
    RELOC_NUMBER (R_ARC_GOTPC32,0x33)
    RELOC_NUMBER (R_ARC_PLT32,0x34)
    RELOC_NUMBER (R_ARC_COPY, 0x35)
    RELOC_NUMBER (R_ARC_GLOB_DAT, 0x36)
    RELOC_NUMBER (R_ARC_JMP_SLOT, 0x37)
    RELOC_NUMBER (R_ARC_RELATIVE, 0x38)
    RELOC_NUMBER (R_ARC_GOTOFF, 0x39)
    RELOC_NUMBER (R_ARC_GOTPC, 0x3A)
    RELOC_NUMBER (R_ARC_GOT32, 0x3B)
END_RELOC_NUMBERS (R_ARC_max)

/* Processor specific flags for the ELF header e_flags field.  */

/* Four bit ARC machine type field.  */
#define EF_ARC_MACH		0x0000000f

/* Various CPU types.  */
#define E_ARC_MACH_A4		0x00000000
#define E_ARC_MACH_A5		0x00000001
#define E_ARC_MACH_ARC600	0x00000002
#define E_ARC_MACH_ARC700	0x00000003


#endif /* _ELF_ARC_H */
