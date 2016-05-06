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
#include <sys/types.h>
#include <unistd.h>

uid_t getuid()
{
	return syscall0(int, SYS_GETUID);
}

uid_t geteuid()
{
	return syscall0(int, SYS_GETEUID);
}

int setuid(uid_t uid)
{
	int ret = syscall1(int, SYS_SETUID, (unsigned int) uid);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}

int seteuid(uid_t euid)
{
	int ret = syscall1(int, SYS_SETEUID, (unsigned int) euid);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}

int setreuid(uid_t ruid, uid_t euid)
{
	int ret = syscall2(int, SYS_SETREUID, (unsigned int) ruid, (unsigned int) euid);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}

gid_t getgid()
{
	return syscall0(int, SYS_GETGID);
}

gid_t getegid()
{
	return syscall0(int, SYS_GETEGID);
}

int setgid(gid_t gid)
{
	int ret = syscall2(int, SYS_SETGID, (unsigned int) gid, 0);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}

int setegid(gid_t egid)
{
	int ret = syscall1(int, SYS_SETEGID, (unsigned int) egid);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}

int setregid(gid_t rgid, gid_t egid)
{
	int ret = syscall2(int, SYS_SETREGID, (unsigned int) rgid, (unsigned int) egid);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}

int setpgid(pid_t pid, pid_t pgid)
{
	int ret = syscall2(int, SYS_SETGID, pgid, pid);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}

pid_t getsid(pid_t pid)
{
	errno = ENOSYS;
	return -1;
}

pid_t setsid()
{
	return getpid();
}
