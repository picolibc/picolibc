/* This is file RESOURCE.H */
/*
** Copyright (C) 1991 DJ Delorie
**
** This file is distributed under the terms listed in the document
** "copying.dj".
** A copy of "copying.dj" should accompany this file; if not, a copy
** should be available from where this file was obtained.  This file
** may not be distributed without a verbatim copy of "copying.dj".
**
** This file is distributed WITHOUT ANY WARRANTY; without even the implied
** warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef _SYS_RESOURCE_H_
#define _SYS_RESOURCE_H_

#include <sys/time.h>

#define	RUSAGE_SELF	0		/* calling process */
#define	RUSAGE_CHILDREN	-1		/* terminated child processes */

struct rusage {
	struct timeval ru_utime;	/* user time used */
	struct timeval ru_stime;	/* system time used */
	long ru_maxrss;			/* integral max resident set size */
	long ru_ixrss;			/* integral shared text memory size */
	long ru_idrss;			/* integral unshared data size */
	long ru_isrss;			/* integral unshared stack size */
	long ru_minflt;			/* page reclaims */
	long ru_majflt;			/* page faults */
	long ru_nswap;			/* swaps */
	long ru_inblock;		/* block input operations */
	long ru_oublock;		/* block output operations */
	long ru_msgsnd;			/* messages sent */
	long ru_msgrcv;			/* messages received */
	long ru_nsignals;		/* signals received */
	long ru_nvcsw;			/* voluntary context switches */
	long ru_nivcsw;			/* involuntary context switches */
};


#ifdef __cplusplus
extern "C" int getrusage(int who, struct rusage *rusage);
#else
extern int getrusage(int who, struct rusage *rusage);
#endif

#endif

