/* sys/statvfs.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _SYS_STATVFS_H_
#define _SYS_STATVFS_H_

#include <sys/types.h>

#define ST_RDONLY 0x80000	/* equals FILE_READ_ONLY_VOLUME */
#define ST_NOSUID 0		/* Looking for that bit should always fail. */

struct statvfs {
   unsigned long    f_bsize;    /* file system block size */
   unsigned long    f_frsize;   /* fragment size */
   fsblkcnt_t	    f_blocks;   /* size of fs in f_frsize units */
   fsblkcnt_t	    f_bfree;    /* free blocks in fs */
   fsblkcnt_t	    f_bavail;   /* free blocks avail to non-superuser */
   fsfilcnt_t	    f_files;    /* total file nodes in file system */
   fsfilcnt_t	    f_ffree;    /* free file nodes in fs */
   fsfilcnt_t	    f_favail;   /* avail file nodes in fs */
   unsigned long    f_fsid;     /* file system id */
   unsigned long    f_flag;	/* mount flags */
   unsigned long    f_namemax;  /* maximum length of filenames */
};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int statvfs (const char *__path, struct statvfs *__buf);
int fstatvfs (int __fd, struct statvfs *__buf);

#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif /*_SYS_STATVFS_H_*/
