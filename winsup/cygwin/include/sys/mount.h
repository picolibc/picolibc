/* sys/mount.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _SYS_MOUNT_H
#define _SYS_MOUNT_H

#define BLOCK_SIZE 1024
#define BLOCK_SIZE_BITS	10

#ifdef __cplusplus
extern "C" {
#endif

enum
{
  MOUNT_SYMLINK =	0x00001,	/* "mount point" is a symlink */
  MOUNT_BINARY =	0x00002,	/* "binary" format read/writes */
  MOUNT_SYSTEM =	0x00008,	/* mount point came from system table */
  MOUNT_EXEC   =	0x00010,	/* Any file in the mounted directory
					   gets 'x' bit */
  MOUNT_CYGDRIVE   =	0x00020,	/* mount point refers to cygdrive
					   device mount */
  MOUNT_CYGWIN_EXEC =	0x00040,	/* file or directory is or contains a
					   cygwin executable */
  MOUNT_SPARSE	=	0x00080,	/* Support automatic sparsifying of
					   files. */
  MOUNT_NOTEXEC =	0x00100,	/* don't check files for executable magic */
  MOUNT_DEVFS =		0x00200,	/* /device "filesystem" */
  MOUNT_PROC =		0x00400,	/* /proc "filesystem" */
  MOUNT_RO =		0x01000,	/* read-only "filesystem" */
  MOUNT_NOACL =		0x02000,	/* support reading/writing ACLs */
  MOUNT_NOPOSIX =	0x04000,	/* Case insensitve path handling */
  MOUNT_OVERRIDE =	0x08000,	/* Allow overriding of root */
  MOUNT_IMMUTABLE =	0x10000,	/* Mount point can't be changed */
  MOUNT_AUTOMATIC =	0x20000,	/* Mount point was added automatically */
  MOUNT_DOS =		0x40000,	/* convert leading spaces and trailing
					   dots and spaces to private use area */
  MOUNT_IHASH =		0x80000,	/* Enforce hash values for inode numbers */
  MOUNT_BIND =		0x100000,	/* Allows bind syntax in fstab file. */
  MOUNT_USER_TEMP =	0x200000	/* Mount the user's $TMP. */
};

int mount (const char *, const char *, unsigned __flags);
int umount (const char *);
int cygwin_umount (const char *__path, unsigned __flags);

#ifdef __cplusplus
};
#endif

#endif /* _SYS_MOUNT_H */
