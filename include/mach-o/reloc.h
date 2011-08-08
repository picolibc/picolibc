/* Mach-O support for BFD.
   Copyright 2011
   Free Software Foundation, Inc.

   This file is part of BFD, the Binary File Descriptor library.

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

#ifndef _MACH_O_RELOC_H
#define _MACH_O_RELOC_H

/* Fields for a normal (non-scattered) entry.  */
#define BFD_MACH_O_R_PCREL		0x01000000
#define BFD_MACH_O_GET_R_LENGTH(s)	(((s) >> 25) & 0x3)
#define BFD_MACH_O_R_EXTERN		0x08000000
#define BFD_MACH_O_GET_R_TYPE(s)	(((s) >> 28) & 0x0f)
#define BFD_MACH_O_GET_R_SYMBOLNUM(s)	((s) & 0x00ffffff)
#define BFD_MACH_O_SET_R_LENGTH(l)	(((l) & 0x3) << 25)
#define BFD_MACH_O_SET_R_TYPE(t)	(((t) & 0xf) << 28)
#define BFD_MACH_O_SET_R_SYMBOLNUM(s)	((s) & 0x00ffffff)

/* Fields for a scattered entry.  */
#define BFD_MACH_O_SR_SCATTERED		0x80000000
#define BFD_MACH_O_SR_PCREL		0x40000000
#define BFD_MACH_O_GET_SR_LENGTH(s)	(((s) >> 28) & 0x3)
#define BFD_MACH_O_GET_SR_TYPE(s)	(((s) >> 24) & 0x0f)
#define BFD_MACH_O_GET_SR_ADDRESS(s)	((s) & 0x00ffffff)
#define BFD_MACH_O_SET_SR_LENGTH(l)	(((l) & 0x3) << 28)
#define BFD_MACH_O_SET_SR_TYPE(t)	(((t) & 0xf) << 24)
#define BFD_MACH_O_SET_SR_ADDRESS(s)	((s) & 0x00ffffff)

/* Generic relocation types (used by i386).  */
#define BFD_MACH_O_GENERIC_RELOC_VANILLA 	0
#define BFD_MACH_O_GENERIC_RELOC_PAIR	 	1
#define BFD_MACH_O_GENERIC_RELOC_SECTDIFF	2
#define BFD_MACH_O_GENERIC_RELOC_PB_LA_PTR	3
#define BFD_MACH_O_GENERIC_RELOC_LOCAL_SECTDIFF	4
#define BFD_MACH_O_GENERIC_RELOC_TLV		5

#endif /* _MACH_O_RELOC_H */
