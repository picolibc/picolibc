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

#include <fcntl.h>
#include <string.h>
#include <sys/dirent.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <unistd.h>

int shm_open(const char *name, int oflag, mode_t mode)
{
	char shm_name[PATH_MAX_SIZE + 24] = "/dev/ipc/shm_";

	if (*name == '/')
	  ++name;

	strncpy(shm_name + 13, name, PATH_MAX_SIZE + 10);

	int fd = open(shm_name, oflag, mode);
	if (fd >= 0) {
		int flags = fcntl(fd, F_GETFD, 0);
		if (flags >= 0) {
			flags |= FD_CLOEXEC;
			flags = fcntl(fd, F_SETFD, flags);
		}

		if (flags == -1) {
			close(fd);
			return -1;
		}
	}

	if (fd != -1 && (oflag | O_CREAT)) {
		if (ioctl(fd, IPC_SHM_INIT, 0) < 0) {
			close(fd);
			return -1;
		}
	}

	return fd;
}

int shm_unlink(const char *name)
{
	return unlink(name);
}
