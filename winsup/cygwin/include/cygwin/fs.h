/* cygwin/fs.h
   Written by Chris January <chris@atomice.net>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGWIN_FS_H_
#define _CYGWIN_FS_H_

#include <asm/socket.h>

#define BLKRRPART    0x0000125f
#define BLKGETSIZE   0x00001260
#define BLKSSZGET    0x00001268
#define BLKIOMIN     0x00001278
#define BLKIOOPT     0x00001279
#define BLKALIGNOFF  0x0000127a
#define BLKPBSZGET   0x0000127b
#define BLKGETSIZE64 0x00041268

/* Get/Set file attributes */
#define FS_IOC_GETFLAGS		_IOR('f', 1, __uint64_t)
#define FS_IOC_SETFLAGS		_IOW('f', 2, __uint64_t)

/* Inode flags (FS_IOC_GETFLAGS / FS_IOC_SETFLAGS)

   This is loosely following the Linux inode flags.  Basically it's just
   a convenient way to handle certain aspects of files on Windows which
   are not covered by POSIX calls, mostly connected to DOS attributes. */
#define FS_READONLY_FL		0x000000001ULL /* DOS R/O */
#define FS_HIDDEN_FL		0x000000002ULL /* DOS Hidden */
#define FS_SYSTEM_FL		0x000000004ULL /* DOS System */
#define FS_ARCHIVE_FL		0x000000020ULL /* DOS Archive */
#define FS_TEMP_FL		0x000000100ULL /* DOS Temporary */
#define FS_SPARSE_FL		0x000000200ULL /* Sparse file */
#define FS_REPARSE_FL		0x000000400ULL /* Reparse point */
#define FS_COMPRESSED_FL	0x000000800ULL /* Compressed file */
#define FS_OFFLINE_FL		0x000001000ULL /* DOS Offline */
#define FS_NOTINDEXED_FL	0x000002000ULL /* DOS Not context indexed */
#define FS_ENCRYPT_FL		0x000004000ULL /* Encrypted file */
#define FS_CASESENS_FL		0x100000000ULL /* Case sensitive dir */

#define FS_FL_USER_VISIBLE	0x100007f27ULL /* User visible flags */
#define FS_FL_USER_MODIFIABLE	0x100006b27ULL /* User modifiable flags */

/* Flags for renameat2, from /usr/include/linux/fs.h.  For now we
   support only RENAME_NOREPLACE. */
#define RENAME_NOREPLACE (1 << 0)
/* #define RENAME_EXCHANGE  (1 << 1) */
/* #define RENAME_WHITEOUT  (1 << 2) */

#endif
