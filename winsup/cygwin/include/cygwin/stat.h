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

#ifdef __INSIDE_CYGWIN__
struct __stat32
{
  dev_t         st_dev;
  ino_t         st_ino;
  mode_t        st_mode;
  nlink_t       st_nlink;
  __uid16_t     st_uid;
  __gid16_t     st_gid;
  dev_t         st_rdev;
  __off32_t     st_size;
  time_t        st_atime;
  long          st_spare1;
  time_t        st_mtime;
  long          st_spare2;
  time_t        st_ctime;
  long          st_spare3;
  blksize_t     st_blksize;
  __blkcnt32_t  st_blocks;
  long          st_spare4[2];
};

struct __stat64
{
  dev_t         st_dev;
  ino_t         st_ino;
  mode_t        st_mode;
  nlink_t       st_nlink;
  __uid32_t     st_uid;
  __gid32_t     st_gid;
  dev_t         st_rdev;
  __off64_t     st_size;
  time_t        st_atime;
  long          st_spare1;
  time_t        st_mtime;
  long          st_spare2;
  time_t        st_ctime;
  long          st_spare3;
  blksize_t     st_blksize;
  __blkcnt64_t  st_blocks;
  long          st_spare4[2];
};

extern int fstat64 (int fd, struct __stat64 *buf);
extern int stat64 (const char *file_name, struct __stat64 *buf);
extern int lstat64 (const char *file_name, struct __stat64 *buf);

#endif

struct stat
{ 
  dev_t         st_dev;
  ino_t         st_ino;
  mode_t        st_mode;
  nlink_t       st_nlink;
  uid_t         st_uid;
  gid_t         st_gid;
  dev_t         st_rdev;
  off_t         st_size;
  time_t        st_atime;
  long          st_spare1;
  time_t        st_mtime;
  long          st_spare2;
  time_t        st_ctime;
  long          st_spare3;
  blksize_t     st_blksize;
  blkcnt_t      st_blocks;
  long          st_spare4[2];
};

#ifdef __cplusplus
}
#endif

#endif /* _CYGWIN_STAT_H */
