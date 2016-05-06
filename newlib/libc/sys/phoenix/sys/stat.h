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

#ifndef _SYS_STAT_H
#define _SYS_STAT_H

#include <phoenix/stat.h>
#include <sys/types.h>

#define	_IFMT		0170000
#define	_IFDIR		0040000
#define	_IFCHR		0020000
#define	_IFBLK		0060000
#define	_IFREG		0100000
#define	_IFLNK		0120000
#define	_IFSOCK		0140000
#define	_IFIFO		0010000

#define S_BLKSIZE	1024

#define	S_IREAD		0000400		/* Read permission, owner. */
#define	S_IWRITE 	0000200		/* Write permission, owner. */
#define	S_IEXEC		0000100		/* Execute/search permission, owner. */
#define	S_ENFMT 	0002000		/* Enforcement-mode locking. */

#define ACCESSPERMS			(S_IRWXU | S_IRWXG | S_IRWXO)								/* 0777 */
#define ALLPERMS			(S_ISUID | S_ISGID | S_ISVTX | S_IRWXU | S_IRWXG | S_IRWXO) /* 07777 */
#define DEFFILEMODE			(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) /* 0666 */

#define S_TYPEISMQ(buf)  	((buf)->st_mode - (buf)->st_mode)
#define S_TYPEISSEM(buf) 	((buf)->st_mode - (buf)->st_mode)
#define S_TYPEISSHM(buf) 	((buf)->st_mode - (buf)->st_mode)

int chmod(const char *pathname, mode_t mode);
int fchmod(int fd, mode_t mode);
int stat(const char *pathname, struct stat *buf);
int fstat(int fd, struct stat *buf);
int lstat(const char *pathname, struct stat *buf);
int	mkdir(const char *pathname, mode_t mode);
int	mkfifo(const char *pathname, mode_t mode);
int	mknod(const char *pathname, mode_t mode, dev_t dev);
mode_t umask(mode_t mask);

#ifdef __LARGE64_FILES
struct stat64;
int	fstat64(int fd, struct stat64 *buf);
#endif

#endif
