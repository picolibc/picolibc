/* Alpha VMS external format of Debug Symbol Table.

   Copyright 2010 Free Software Foundation, Inc.
   Written by Tristan Gingold <gingold@adacore.com>, AdaCore.

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

#ifndef _VMS_DST_H
#define _VMS_DST_H

/* Also available in vms freeware v5.0 debug/alpha_dstrecrds.sdl.  */

struct vms_dst_header
{
  /* Length.  */
  unsigned char length[2];

  /* Type.  */
  unsigned char type[2];
};

/* Beginning of module.  */
#define DST__K_MODBEG 188

/* Some well known languages.  */
#define DST__K_MACRO 0
#define DST__K_BLISS 2
#define DST__K_C 7
#define DST__K_ADA 9

struct vms_dst_modbeg
{
  unsigned char flags;
  unsigned char unused;
  unsigned char language[4];
  unsigned char major[2];
  unsigned char minor[2];
  /* Module name ASCIC.  */
  /* Ident name ASCIC.  */
};

/* Routine begin.  */
#define DST__K_RTNBEG 190

struct vms_dst_rtnbeg
{
  unsigned char flags;

  /* Address of the code.  */
  unsigned char address[4];

  /* Procedure descriptor address.  */
  unsigned char pd_address[4];

  /* Name: ASCIC  */
};

/* Line number.  */
#define DST__K_LINE_NUM 185

struct vms_dst_pcline
{
  unsigned char pcline_command;
  unsigned char field[4];
};

#define DST__K_DELTA_PC_W 1
#define DST__K_INCR_LINUM 2
#define DST__K_INCR_LINUM_W 3
#define DST__K_SET_LINUM  9
#define DST__K_SET_LINUM_B 19
#define DST__K_SET_LINUM_L 20
#define DST__K_TERM 14
#define DST__K_SET_ABS_PC 16
#define DST__K_DELTA_PC_L 17

/* Routine end.  */
#define DST__K_RTNEND 191

struct vms_dst_rtnend
{
  unsigned char unused;
  unsigned char size[4];
};

/* Prologue.  */
#define DST__K_PROLOG 162

struct vms_dst_prolog
{
  unsigned char bkpt_addr[4];
};

/* Epilog.  */
#define DST__K_EPILOG 127

struct vms_dst_epilog
{
  unsigned char flags;
  unsigned char count[4];
};

/* Module end.  */
#define DST__K_MODEND 189

/* Block begin.  */
#define DST__K_BLKBEG 176

struct vms_dst_blkbeg
{
  unsigned char unused;
  unsigned char address[4];
  /* Name ASCIC.  */
};

/* Block end.  */
#define DST__K_BLKEND 177

struct vms_dst_blkend
{
  unsigned char unused;
  unsigned char size[4];
};

/* Source correlation.  */
#define DST__K_SOURCE 155

#define DST__K_SRC_DECLFILE    1
#define DST__K_SRC_SETFILE     2
#define DST__K_SRC_SETREC_L    3
#define DST__K_SRC_SETREC_W    4
#define DST__K_SRC_SETLNUM_L   5
#define DST__K_SRC_SETLNUM_W   6
#define DST__K_SRC_DEFLINES_W 10
#define DST__K_SRC_DEFLINES_B 11
#define DST__K_SRC_FORMFEED   16

struct vms_dst_src_decl_src
{
  unsigned char length;
  unsigned char flags;
  unsigned char fileid[2];
  unsigned char rms_cdt[8];
  unsigned char rms_ebk[4];
  unsigned char rms_ffb[2];
  unsigned char rms_rfo;
  /* Filename ASCIC.  */
};

#endif /* _VMS_DST_H */
