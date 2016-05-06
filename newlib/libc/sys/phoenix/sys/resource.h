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

#ifndef _SYS_RESOURCE_H
#define _SYS_RESOURCE_H

#include <sys/time.h>
#include <sys/types.h>

#define	RUSAGE_SELF			0
#define	RUSAGE_CHILDREN		(-1)
#define RUSAGE_BOTH			(-2)	/* sys_wait4() uses this */
#define	RUSAGE_THREAD		1		/* Only the calling thread */

struct rusage {
	struct timeval ru_utime;		/* User time used */
	struct timeval ru_stime;		/* System time used */
	long ru_maxrss;					/* Maximum resident set size */
	long ru_ixrss;					/* Integral shared memory size */
	long ru_idrss;					/* Integral unshared data size */
	long ru_isrss;					/* Integral unshared stack size */
	long ru_minflt;					/* Page reclaims */
	long ru_majflt;					/* Page faults */
	long ru_nswap;					/* Swaps */
	long ru_inblock;				/* Block input operations */
	long ru_oublock;				/* Block output operations */
	long ru_msgsnd;					/* Messages sent */
	long ru_msgrcv;					/* Messages received */
	long ru_nsignals;				/* Signals received */
	long ru_nvcsw;					/* Voluntary context switches */
	long ru_nivcsw;					/* Involuntary */
};

typedef unsigned long rlim_t;

struct rlimit {
	rlim_t rlim_cur;
	rlim_t rlim_max;
};

#define RLIM64_INFINITY		(~0ULL)

struct rlimit64 {
	uint64_t rlim_cur;
	uint64_t rlim_max;
};

#define	PRIO_MIN			(-20)
#define	PRIO_MAX			20
#define	PRIO_PROCESS		0
#define	PRIO_PGRP			1
#define	PRIO_USER			2

/*
 * Limit the stack by to some sane default: root can always
 * increase this limit if needed. 8MB seems reasonable.
 */
#define _STK_LIM			(8 * 1024 * 1024)

/*
 * GPG2 wants 64kB of mlocked memory, to make sure pass phrases
 * and other sensitive information are never written to disk.
 */
#define MLOCK_LIMIT			((PAGE_SIZE > 64 * 1024) ? PAGE_SIZE : 64 * 1024)

#define RLIMIT_CPU			0	/* CPU time in sec */
#define RLIMIT_FSIZE		1	/* Maximum filesize */
#define RLIMIT_DATA			2	/* Max data size */
#define RLIMIT_STACK		3	/* Max stack size */
#define RLIMIT_CORE			4	/* Max core file size */
#define RLIMIT_RSS			5	/* Max resident set size */
#define RLIMIT_NPROC		6	/* Max number of processes */
#define RLIMIT_NOFILE		7	/* Max number of open files */
#define RLIMIT_OFILE		RLIMIT_NOFILE
#define RLIMIT_MEMLOCK		8	/* Max locked-in-memory address space */
#define RLIMIT_AS			9	/* Address space limit */
#define RLIMIT_LOCKS		10	/* Maximum file locks held */
#define RLIMIT_SIGPENDING	11	/* Max number of pending signals */
#define RLIMIT_MSGQUEUE		12	/* Maximum bytes in POSIX mqueues */
#define RLIMIT_NICE			13	/* Max nice prio allowed to raise to 0-39 for nice level 19 .. -20 */
#define RLIMIT_RTPRIO		14	/* Maximum realtime priority */
#define RLIMIT_RTTIME		15	/* Timeout for RT tasks in us */
#define RLIM_NLIMITS		16

#ifndef RLIM_INFINITY
#define RLIM_INFINITY		(~0UL)
#endif

/* RLIMIT_STACK default maximum - some architectures override it. */
#ifndef _STK_LIM_MAX
#define _STK_LIM_MAX		RLIM_INFINITY
#endif

int getpriority(int which, id_t who);
int setpriority(int which, id_t who, int prio);
int getrlimit(int resource, struct rlimit *rlim);
int setrlimit(int resource, const struct rlimit *rlim);
int getrusage(int who, struct rusage *usage);

#endif
