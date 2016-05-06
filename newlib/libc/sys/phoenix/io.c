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
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>

int open(const char *pathname, int flags, ...)
{
	int mode = S_IFREG | S_IRWXUGO;

	if (flags & O_CREAT) {
		va_list arg;
		va_start(arg, flags);
		mode = va_arg(arg, int);
		va_end(arg);
	}

	int ret = syscall3(int, SYS_OPEN, pathname, flags, mode);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}

ssize_t read(int fd, void *buf, size_t count)
{
	int ret = syscall3(int, SYS_READ, fd, buf, count);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}

ssize_t write(int fd, const void *buf, size_t count)
{
	int ret = syscall3(int, SYS_WRITE, fd, buf, count);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}

off_t lseek(int fd, off_t offset, int whence)
{
	int ret = syscall3(int, SYS_LSEEK, fd, offset, whence);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}

int close(int fd)
{
	int ret = syscall1(int, SYS_CLOSE, fd);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}

int _close(int fd)
{
	return close(fd);
}
