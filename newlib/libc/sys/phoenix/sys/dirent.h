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

#ifndef _SYS_DIRENT_H
#define _SYS_DIRENT_H

#include <phoenix/fs.h>
#include <sys/lock.h>
#include <sys/types.h>

#define PATH_MAX_SIZE		1024

#define HAVE_DD_LOCK

typedef struct {
	int dd_fd;		/* Directory file. */
	int dd_loc;		/* Position in buffer. */
	int dd_seek;
	char *dd_buf;	/* Pointer to buffer. */
	int dd_len;		/* Buffer size. */
	int dd_size;	/* Data size in buffer. */
	_LOCK_RECURSIVE_T dd_lock;
} DIR;

#define __dirfd(dir)	(dir)->dd_fd

DIR *opendir(const char *name);
struct dirent *readdir(DIR *dirp);
void rewinddir(DIR *dirp);
int closedir(DIR *dirp);

long telldir(DIR *dirp);
void seekdir(DIR *dirp, off_t loc);
int scandir(const char *__dir,
            struct dirent ***__namelist,
            int (*select) (const struct dirent *),
            int (*compar) (const struct dirent **, const struct dirent **));
int alphasort(const struct dirent **__a, const struct dirent **__b);

#define _seekdir		seekdir

/* Declare which dirent fields are available in Phoenix-RTOS. */
#undef _DIRENT_HAVE_D_NAMLEN
#define _DIRENT_HAVE_D_OFF
#define _DIRENT_HAVE_D_RECLEN

#define d_fileno		d_ino		/* Backwards compatibility. */

#ifdef __USE_LARGEFILE64
#define dirent64		dirent
#endif

#endif
