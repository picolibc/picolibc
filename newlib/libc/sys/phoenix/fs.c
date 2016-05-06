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

#include "syscall.h"

#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <stdio.h>
#include <sys/dirent.h>
#include <sys/stat.h>

#define MAX_PATH		1024

int mkdir(const char *pathname, mode_t mode)
{
	int ret = syscall2(int, SYS_MKDIR, pathname, mode);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}

int rmdir(const char *pathname)
{
	int ret = syscall1(int, SYS_RMDIR, pathname);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}

int chdir(const char *path)
{
	int ret = syscall1(int, SYS_SETCWD, path);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}

char *getcwd(char *buf, size_t size)
{	
	if (buf == NULL)
		buf = (char *) malloc(MAX_PATH);

	int ret = syscall2(int, SYS_GETCWD, buf, size);
	if (ret < 0) {
		errno = -ret;
		return NULL;
	}

	return buf;
}

int unlink(const char *pathname)
{
	int ret = syscall1(int, SYS_UNLINK, pathname);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}

int	mkfifo(const char *pathname, mode_t mode)
{
	int ret = syscall3(int, SYS_OPEN, pathname, O_CREAT, (mode | S_IFIFO));
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}

int	mknod(const char *pathname, mode_t mode, dev_t dev)
{
	int ret = syscall3(int, SYS_MKNOD, pathname, mode, dev);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}

int getdents(unsigned int fd, struct dirent *dirp, unsigned int count)
{
	int ret = syscall3(int, SYS_GETDENTS, fd, dirp, count);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}

int chroot(const char *path)
{
	return chdir(path);
}
