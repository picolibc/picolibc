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
#include <signal.h>
#include <sys/types.h>

int kill(pid_t pid, int sig)
{
	int ret = syscall2(int, SYS_KILL, pid, sig);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}

sighandler_t signal(int signum, sighandler_t handler)
{
	/* TODO: implement. */
	errno = ENOSYS;
	return SIG_ERR;
}

int sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
	/* TODO: implement. */
	errno = ENOSYS;
	return -1;
}

int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact)
{
	/* TODO: implement. */
	errno = ENOSYS;
	return -1;
}

int sigsuspend(const sigset_t *mask)
{
	/* TODO: implement. */
	errno = ENOSYS;
	return -1;
}

int raise(int sig)
{
	/* TODO: implement. */
	errno = ENOSYS;
	return -1;
}
