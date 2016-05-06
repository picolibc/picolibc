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
#include <sys/times.h>
#include <time.h>

/* Global variables. */
int _daylight = 0;
long _timezone = 0;
char *_tzname[2] = {"GMT", "GMT"};

time_t time(time_t *t)
{
	struct timespec tp;
	int ret = clock_gettime(CLOCK_REALTIME, &tp);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	if (t != NULL)
		*t = tp.tv_sec;

	return tp.tv_sec;
}

int stime(const time_t *t)
{
	if (t == NULL) {
		errno = EINVAL;
		return -1;
	}

	struct timespec tp;
	tp.tv_sec = *t;

	int ret = clock_settime(CLOCK_REALTIME, &tp);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}

int gettimeofday(struct timeval *tv, void *tz)
{
	struct timespec tp;
	int ret = clock_gettime(CLOCK_REALTIME, &tp);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	tv->tv_sec = tp.tv_sec;
	tv->tv_usec = tp.tv_nsec / 1000;

	return ret;
}

int settimeofday(const struct timeval *tv, const struct timezone *tz)
{
	struct timespec tp;
	tp.tv_sec = tv->tv_sec;
	tp.tv_nsec = tv->tv_usec * 1000;

	int ret = clock_settime(CLOCK_REALTIME, &tp);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}

clock_t times(struct tms *buf)
{
	errno = ENOSYS;
	return -1;
}
