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

#include <sys/sched.h>
#include <sys/types.h>

int sched_getparam(pid_t pid, struct sched_param *param)
{
	int ret = syscall2(int, SYS_SCHED_GETPARAM, pid, param);
	if(ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}

int sched_setparam(pid_t pid, const struct sched_param *param)
{
	int ret = syscall2(int, SYS_SCHED_SETPARAM, pid, param);
	if(ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}

int sched_getscheduler(pid_t pid)
{
	int ret = syscall1(int, SYS_SCHED_GETSCHEDULER, pid);
	if(ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}

int sched_setscheduler(pid_t pid, int policy, const struct sched_param *param)
{
	int ret = syscall3(int, SYS_SCHED_SETSCHEDULER, pid, policy, param);
	if(ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}

int sched_get_priority_max(int policy)
{
	int ret = syscall1(int, SYS_SCHED_GET_PRIORITY_MAX, policy);
	if(ret < 0) {
		errno = -ret;
		return -1;
	}
	
	return ret;
}

int sched_get_priority_min(int policy)
{
	int ret = syscall1(int, SYS_SCHED_GET_PRIORITY_MIN, policy);
	if(ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}

int sched_yield(void)
{
	int ret = syscall0(int, SYS_SCHED_YIELD);
	if(ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}
