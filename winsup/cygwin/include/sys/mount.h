/* sys/mount.h

   Copyright 1998, 1999, 2000, 2001 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _SYS_MOUNT_H
#define _SYS_MOUNT_H

#ifdef __cplusplus
extern "C" {
#endif

enum
{
  MOUNT_SYMLINK =	0x0001,	/* "mount point" is a symlink */
  MOUNT_BINARY =	0x0002,	/* "binary" format read/writes */
  MOUNT_SYSTEM =	0x0008,	/* mount point came from system table */
  MOUNT_EXEC   =	0x0010,	/* Any file in the mounted directory gets 'x' bit */
  MOUNT_CYGDRIVE   =	0x0020,	/* mount point refers to cygdrive device mount */
  MOUNT_CYGWIN_EXEC =	0x0040,	/* file or directory is or contains a cygwin
				   executable */
  MOUNT_MIXED	=	0x0080,	/* reads are text, writes are binary
				   not yet implemented */
  MOUNT_NOTEXEC =	0x0100,	/* don't check files for executable magic */
  MOUNT_DEVFS =		0x0200,	/* /device "filesystem" */
  MOUNT_PROC =		0x0400,	/* /proc "filesystem" */
  MOUNT_ENC =		0x0800,	/* encode special characters */
  MOUNT_RO =		0x1000  /* read-only "filesystem" */
};

int mount (const char *, const char *, unsigned __flags);
int umount (const char *);
int cygwin_umount (const char *__path, unsigned __flags);

#ifdef __cplusplus
};
#endif

#endif /* _SYS_MOUNT_H */
