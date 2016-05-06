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
#include <malloc.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
	void *ret = NULL;
	MmapArg_t arg = {
		.fd = fd,
		.addr = addr,
		.pflags = prot,
		.mflags = flags,
		.offs = offset,
		.len = length,
		.raddr = &ret
	};

	struct stat st;
	if (fstat(fd, &st) < 0)
		return NULL;

	if (st.st_mode & S_IFREG) {
		if (prot != PROT_READ) {
			errno = ENOSYS;
			return MAP_FAILED;
		}

		if ((ret = malloc(length)) == 0) {
			errno = ENOMEM;
			return MAP_FAILED;
		}

		if (pread(fd, ret, length, offset) < 0) {
			free(ret);
			return MAP_FAILED;
		}
	}
	else {
		int err = syscall1(int, SYS_MMAP, &arg);
		if(err < 0) {
			errno = -err;
			return MAP_FAILED;
		}
	}

	return ret;
}

int munmap(void *addr, size_t length)
{
	int ret = syscall2(int, SYS_MUNMAP, addr, length);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}
