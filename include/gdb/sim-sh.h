/* This file defines the interface between the sh simulator and gdb.
   Copyright (C) 2002 Free Software Foundation, Inc.

This file is part of GDB.

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

#if !defined (SIM_SH_H)
#define SIM_SH_H

#ifdef __cplusplus
extern "C" { // }
#endif

/* The simulator makes use of the following register information. */

enum
  {
    SIM_SH64_R0_REGNUM = 0,
    SIM_SH64_SP_REGNUM = 15,
    SIM_SH64_PC_REGNUM = 64,
    SIM_SH64_SR_REGNUM = 65,
    SIM_SH64_SSR_REGNUM = 66,
    SIM_SH64_SPC_REGNUM = 67,
    SIM_SH64_TR0_REGNUM = 68,
    SIM_SH64_FPCSR_REGNUM = 76,
    SIM_SH64_FR0_REGNUM = 77
  };

enum
  {
    SIM_SH64_NR_REGS = 141,  /* total number of architectural registers */
    SIM_SH64_NR_R_REGS = 64, /* number of general registers */
    SIM_SH64_NR_TR_REGS = 8, /* number of target registers */
    SIM_SH64_NR_FP_REGS = 64 /* number of floating point registers */
  };

#ifdef __cplusplus
}
#endif

#endif
