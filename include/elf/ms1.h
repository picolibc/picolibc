/* MS1  ELF support for BFD.
   Copyright (C) 2000, 2005 Free Software Foundation, Inc.

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
along with this program; if not, write to the Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#ifndef _ELF_MS1_H
#define _ELF_MS1_H

#include "elf/reloc-macros.h"

/* Relocations.  */
START_RELOC_NUMBERS (elf_ms1_reloc_type)
  RELOC_NUMBER (R_MS1_NONE, 0)
  RELOC_NUMBER (R_MS1_16, 1)
  RELOC_NUMBER (R_MS1_32, 2)
  RELOC_NUMBER (R_MS1_32_PCREL, 3)
  RELOC_NUMBER (R_MS1_PC16, 4)
  RELOC_NUMBER (R_MS1_HI16, 5)
  RELOC_NUMBER (R_MS1_LO16, 6)
END_RELOC_NUMBERS(R_MS1_max)

#define EF_MS1_CPU_MRISC	0x00000001	/* default */
#define EF_MS1_CPU_MRISC2	0x00000002	/* MRISC2 */
#define EF_MS1_CPU_MASK		0x00000003	/* specific cpu bits */
#define EF_MS1_ALL_FLAGS	(EF_MS1_CPU_MASK)

/* The location of the memory mapped hardware stack.  */
#define MS1_STACK_VALUE 0x0f000000
#define MS1_STACK_SIZE  0x20

#endif /* _ELF_MS1_H */
