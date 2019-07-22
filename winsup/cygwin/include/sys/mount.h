/* sys/mount.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _SYS_MOUNT_H
#define _SYS_MOUNT_H

#ifdef __CYGWIN__	/* Build tweak for native Cygwin utils */
#include <cygwin/bits.h>
#endif

#define BLOCK_SIZE 1024
#define BLOCK_SIZE_BITS	10

#ifdef __cplusplus
extern "C" {
#endif

enum
{
  MOUNT_TEXT =		_BIT ( 0),	/* "text" format read/writes */
  MOUNT_SYSTEM =	_BIT ( 3),	/* mount point came from system table */
  MOUNT_EXEC   =	_BIT ( 4),	/* Any file in the mounted directory
					   gets 'x' bit */
  MOUNT_CYGDRIVE   =	_BIT ( 5),	/* mount point refers to cygdrive
					   device mount */
  MOUNT_CYGWIN_EXEC =	_BIT ( 6),	/* file or directory is or contains a
					   cygwin executable */
  MOUNT_SPARSE	=	_BIT ( 7),	/* Support automatic sparsifying of
					   files. */
  MOUNT_NOTEXEC =	_BIT ( 8),	/* don't check files for executable magic */
  MOUNT_DEVFS =		_BIT ( 9),	/* /device "filesystem" */
  MOUNT_PROC =		_BIT (10),	/* /proc "filesystem" */
  MOUNT_RO =		_BIT (12),	/* read-only "filesystem" */
  MOUNT_NOACL =		_BIT (13),	/* support reading/writing ACLs */
  MOUNT_NOPOSIX =	_BIT (14),	/* Case insensitve path handling */
  MOUNT_OVERRIDE =	_BIT (15),	/* Allow overriding of root */
  MOUNT_IMMUTABLE =	_BIT (16),	/* Mount point can't be changed */
  MOUNT_AUTOMATIC =	_BIT (17),	/* Mount point was added automatically */
  MOUNT_DOS =		_BIT (18),	/* convert leading spaces and trailing
					   dots and spaces to private use area */
  MOUNT_IHASH =		_BIT (19),	/* Enforce hash values for inode numbers */
  MOUNT_BIND =		_BIT (20),	/* Allows bind syntax in fstab file. */
  MOUNT_USER_TEMP =	_BIT (21),	/* Mount the user's $TMP. */
  MOUNT_DONT_USE =	_BIT (31)	/* conversion to signed happens. */
};

int mount (const char *, const char *, unsigned __flags);
int umount (const char *);
int cygwin_umount (const char *__path, unsigned __flags);

#ifdef __cplusplus
};
#endif

#endif /* _SYS_MOUNT_H */
