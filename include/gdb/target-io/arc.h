/* Hosted File I/O interface definitions, for GDB, the GNU Debugger.

   Copyright (C) 2007 Free Software Foundation, Inc.

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

#ifndef GDB_TGTIO_ARC_H_
#define GDB_TGTIO_ARC_H_

/* This describes struct stat as seen by the simulator on the host,
   in order to fill in the fields as they are expected by an arc target.  */

#ifdef __GNUC__
#define TGTIO_PACKED __attribute__ ((packed))
#endif

struct fio_arc_stat {
  unsigned long
    tgt_st_dev	: 32,
    tgt_st_ino	: 32,
    tgt_st_mode	: 16,
    tgt_st_nlink: 16,
    tgt_st_uid	: 16,
    tgt_st_gid	: 16,
    tgt_st_rdev	: 32,
    tgt_st_size	: 32,
    tgt_st_blksize: 32,
    tgt_st_blocks: 32;
  long long tgt_st_atime : 64 TGTIO_PACKED;
  long long tgt_st_mtime : 64 TGTIO_PACKED;
  long long tgt_st_ctime : 64 TGTIO_PACKED;
  char tgt_st_reserved[8];
};

/* Likewise for struct timeval.  */
struct fio_timeval
{
  long long tgt_tv_sec : 64 TGTIO_PACKED;
  long tgt_tv_usec : 32;
};
#endif /* GDB_TGTIO_ARC_H_ */
