/* cygwin/stat.h

   Copyright 2002 Red Hat Inc.
   Written by Corinna Vinschen <corinna@vinschen.de>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGWIN_STAT_H
#define _CYGWIN_STAT_H

#ifdef __cplusplus
extern "C" {
#endif

struct  stat
{ 
  dev_t     st_dev;
  ino_t     st_ino;
  mode_t    st_mode;
  nlink_t   st_nlink;
  __uid16_t __st_uid16;
  __gid16_t __st_gid16;
  dev_t     st_rdev;
  __off32_t __st_size32;
  time_t    st_atime;
  __uid32_t __st_uid32;
  time_t    st_mtime;
  __uid32_t __st_gid32;
  time_t    st_ctime;
  long      st_spare3;
  long      st_blksize;
  long      st_blocks;
  __off64_t __st_size64;
};

#ifdef __CYGWIN_USE_BIG_TYPES__
#define st_uid  __st_uid32
#define st_gid  __st_gid32
#define st_size __st_size64
#else
#define st_uid  __st_uid16
#define st_gid  __st_gid16
#define st_size __st_size32
#endif /* __CYGWIN_USE_BIG_TYPES__ */

#ifdef __cplusplus
}
#endif

#endif /* _CYGWIN_STAT_H */
