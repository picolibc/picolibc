/*
Copyright (c) 1982, 1986, 1993
The Regents of the University of California.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
3. Neither the name of the University nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
 */
#ifndef _SYS_RESOURCE_H_
#define _SYS_RESOURCE_H_

#include <sys/cdefs.h>
#include <sys/_types.h>
#include <sys/_timeval.h>

_BEGIN_STD_C

#ifndef _RLIM_T_DECLARED
typedef __rlim_t rlim_t;
#define _RLIM_T_DECLARED
#endif

#ifndef _ID_T_DECLARED
typedef __id_t id_t;
#define _ID_T_DECLARED
#endif

struct rlimit {
    rlim_t rlim_cur;
    rlim_t rlim_max;
};

struct rusage {
    struct timeval ru_utime; /* user time used */
    struct timeval ru_stime; /* system time used */
};

#define RLIM_INFINITY   __INT64_MAX__
#define RLIM_SAVED_MAX  RLIM_INFINITY
#define RLIM_SAVED_CUR  RLIM_INFINITY

#define RUSAGE_SELF     0  /* calling process */
#define RUSAGE_CHILDREN -1 /* terminated child processes */
#if __GNU_VISIBLE
#define RUSAGE_THREAD 1
#endif

#define PRIO_PROCESS  0
#define PRIO_PGRP     1
#define PRIO_USER     2

#define RLIMIT_CPU    0 /* Limit on CPU time per process. */
#define RLIMIT_FSIZE  1 /* Limit on file size. */
#define RLIMIT_DATA   2 /* Limit on data segment size. */
#define RLIMIT_STACK  3 /* Limit on stack size. */
#define RLIMIT_CORE   4 /* Limit on size of core image. */
#define RLIMIT_NOFILE 5 /* Limit on number of open files. */
#define RLIMIT_AS     6 /* Limit on address space size. */

#if __XSI_VISIBLE
int getpriority(int, id_t);
#endif
int getrlimit(int, struct rlimit *);
#if __XSI_VISIBLE
int getrusage(int, struct rusage *);
int setpriority(int, id_t, int);
#endif
int setrlimit(int, const struct rlimit *);

_END_STD_C

#endif /* !_SYS_RESOURCE_H_ */
