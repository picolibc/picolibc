/*
 * hl_fstat.c -- provide _fstat().
 *
 * Copyright (c) 2024 Synopsys Inc.
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions.  No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 *
 */

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "hl_toolchain.h"
#include "hl_api.h"

/* Hostlink IO version of struct stat.  */
struct _hl_stat {
  uint32_t  hl_dev;    /* ID of device containing file.  */
  uint16_t  hl_ino;    /* inode number.  */
  uint16_t  hl_mode;   /* File type and access mode.  */
  int16_t   hl_nlink;  /* Number of hard links.  */
  int16_t   hl_uid;    /* Owner's UID.  */
  int16_t   hl_gid;    /* Owner's GID.  */
  uint8_t   hl_pad[2]; /* Padding to match simulator struct.  */
  uint32_t  hl_rdev;   /* Device ID (if special file).  */
  int32_t   hl_size;   /* Size in bytes.  */
  int32_t   hl_atime;  /* Access time.  */
  int32_t   hl_mtime;  /* Modification time.  */
  int32_t   hl_ctime;  /* Creation time.  */
} __packed;

/* Map Hostlink version of stat struct into newlib's one.  */
static __always_inline void
_hl_fstat_map (const struct _hl_stat *hl_stat, struct stat *stat)
{
  stat->st_dev = hl_stat->hl_dev;
  stat->st_ino = hl_stat->hl_ino;
  stat->st_mode = hl_stat->hl_mode;
  stat->st_nlink = hl_stat->hl_nlink;
  stat->st_uid = hl_stat->hl_uid;
  stat->st_gid = hl_stat->hl_gid;
  stat->st_rdev = hl_stat->hl_rdev;
  stat->st_size = hl_stat->hl_size;
  stat->st_atime = hl_stat->hl_atime;
  stat->st_mtime = hl_stat->hl_mtime;
  stat->st_ctime = hl_stat->hl_ctime;
}

/* Get host file info.  Implements HL_GNUIO_EXT_FSTAT.  */
static __always_inline int
_hl_fstat (int fd, struct stat *buf)
{
  struct _hl_stat hl_stat;
  int32_t ret;
  uint32_t host_errno;

  /* Special version of hostlink - retuned values are passed
   * through inargs.
   */
  host_errno = _user_hostlink (HL_GNUIO_EXT_VENDOR_ID, HL_GNUIO_EXT_FSTAT,
			       "iii",
			       (uint32_t) fd,
			       (uint32_t) &hl_stat,
			       (uint32_t) &ret);

  if (ret < 0)
    {
      errno = host_errno;
      return ret;
    }

  _hl_fstat_map (&hl_stat, buf);

  _hl_delete ();

  return ret;
}

int
_fstat (int fd, struct stat *buf)
{
  return _hl_fstat (fd, buf);
}
