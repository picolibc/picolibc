/* Copyright (c) 2016 Phoenix Systems
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
   OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
   OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
   SUCH DAMAGE.*/

#ifndef	MNTENT_H
#define	MNTENT_H

#include <features.h>
#define __need_FILE
#include <stdio.h>
#include <paths.h>

/* File listing canonical interesting mount points. */
#define	MNTTAB				_PATH_MNTTAB	/* Deprecated alias. */

/* File listing currently active mount points. */
#define	MOUNTED				_PATH_MOUNTED	/* Deprecated alias. */

/* General filesystem types. */
#define MNTTYPE_IGNORE		"ignore"		/* Ignore this entry. */
#define MNTTYPE_NFS			"nfs"			/* Network file system. */
#define MNTTYPE_SWAP		"swap"			/* Swap device. */

/* Generic mount options. */
#define MNTOPT_DEFAULTS		"defaults"		/* Use all default options. */
#define MNTOPT_RO			"ro"			/* Read only. */
#define MNTOPT_RW			"rw"			/* Read/write. */
#define MNTOPT_SUID			"suid"			/* Set uid allowed. */
#define MNTOPT_NOSUID		"nosuid"		/* No set uid allowed. */
#define MNTOPT_NOAUTO		"noauto"		/* Do not auto mount. */

/* Structure describing a mount table entry. */
struct mntent {
	char *mnt_fsname;	/* Device or server for filesystem. */
	char *mnt_dir;		/* Directory mounted on. */
	char *mnt_type;		/* Type of filesystem: ufs, nfs, etc. */
	char *mnt_opts;		/* Comma-separated options for fs. */
	int mnt_freq;		/* Dump frequency (in days). */
	int mnt_passno;		/* Pass number for `fsck'. */
};

FILE *setmntent(const char *filename, const char *type);
struct mntent *getmntent(FILE *fp);
int endmntent(FILE * filep);

#endif
