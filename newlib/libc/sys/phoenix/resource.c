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
#include <sched.h>
#include <sys/resource.h>
#include <sys/sched.h>
#include <sys/types.h>

int getpriority(int which, id_t who)
{
	struct sched_param param;
	if (which != PRIO_PROCESS) {
		errno = ENOSYS;
		return -1;
	}

	int ret = sched_getparam(who, &param);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	errno = EOK;
	return param.sched_priority;
}

int setpriority(int which, id_t who, int prio)
{
	struct sched_param param;
	if (which != PRIO_PROCESS) {
		errno = ENOSYS;
		return -1;
	}

	param.sched_priority = prio;
	int ret = sched_setparam(who, &param);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}

int getrlimit(int resource, struct rlimit *rlim)
{
	/* TODO: implement. */
	errno = ENOSYS;
	return -1;
}

int setrlimit(int resource, const struct rlimit *rlim)
{
	/* TODO: implement. */
	errno = ENOSYS;
	return -1;
}

int getrusage(int who, struct rusage *usage)
{
	/* TODO: implement. */
	errno = ENOSYS;
	return -1;
}
