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

#ifndef	_POSIX_OPT_H
#define	_POSIX_OPT_H

/* Job control is supported. */
#define	_POSIX_JOB_CONTROL	1

/* Processes have a saved set-user-ID and a saved set-group-ID. */
#define	_POSIX_SAVED_IDS	1

/* Priority scheduling is supported. */
#define	_POSIX_PRIORITY_SCHEDULING	1

/* Synchronizing file data is supported. */
#define	_POSIX_SYNCHRONIZED_IO	1

/* The fsync function is present. */
#define	_POSIX_FSYNC	1

/* Mapping of files to memory is supported. */
#define	_POSIX_MAPPED_FILES	1

/* Locking of all memory is supported. */
#define	_POSIX_MEMLOCK	1

/* Locking of ranges of memory is supported. */
#define	_POSIX_MEMLOCK_RANGE	1

/* Setting of memory protections is supported. */
#define	_POSIX_MEMORY_PROTECTION	1

/* Implementation supports `poll' function. */
#define	_POSIX_POLL	1

/* Implementation supports `select' and `pselect' functions. */
#define	_POSIX_SELECT	1

/* Only root can change owner of file. */
#define	_POSIX_CHOWN_RESTRICTED	1

/* `c_cc' member of 'struct termios' structure can be disabled by
   using the value _POSIX_VDISABLE. */
#define	_POSIX_VDISABLE	'\0'

/* Filenames are not silently truncated. */
#define	_POSIX_NO_TRUNC	1

/* X/Open realtime support is available. */
#define _XOPEN_REALTIME	1

/* X/Open realtime thread support is available. */
#define _XOPEN_REALTIME_THREADS	1

/* XPG4.2 shared memory is supported. */
#define	_XOPEN_SHM	1

/* Tell we don't have POSIX threads. */
#undef _POSIX_THREADS

/* We have the reentrant functions described in POSIX. */
#define _POSIX_REENTRANT_FUNCTIONS	1
#define _POSIX_THREAD_SAFE_FUNCTIONS	1

/* We provide priority scheduling for threads. */
#define	_POSIX_THREAD_PRIORITY_SCHEDULING	1

/* We support user-defined stack sizes. */
#define _POSIX_THREAD_ATTR_STACKSIZE	1

/* We support user-defined stacks. */
#define _POSIX_THREAD_ATTR_STACKADDR	1

/* We support POSIX.1b semaphores, but only the non-shared form for now. */
#define _POSIX_SEMAPHORES	1

/* Real-time signals are supported. */
#define _POSIX_REALTIME_SIGNALS	1

/* We support asynchronous I/O. */
#define _POSIX_ASYNCHRONOUS_IO	1
#define _POSIX_ASYNC_IO	1
/* Alternative name for Unix98. */
#define _LFS_ASYNCHRONOUS_IO	1

/* The LFS support in asynchronous I/O is also available. */
#define _LFS64_ASYNCHRONOUS_IO	1

/* The rest of the LFS is also available. */
#define _LFS_LARGEFILE	1
#define _LFS64_LARGEFILE	1
#define _LFS64_STDIO	1

/* POSIX shared memory objects are implemented. */
#define _POSIX_SHARED_MEMORY_OBJECTS	1

/* GNU libc provides regular expression handling. */
#define _POSIX_REGEXP	1

/* Reader/Writer locks are available. */
#define _POSIX_READER_WRITER_LOCKS	200912L

/* We have a POSIX shell. */
#define _POSIX_SHELL	1

/* We support the Timeouts option. */
#define _POSIX_TIMEOUTS	200912L

/* We support spinlocks. */
#define _POSIX_SPIN_LOCKS	200912L

/* The `spawn' function family is supported. */
#define _POSIX_SPAWN	200912L

/* We don't have POSIX timers. */
#undef _POSIX_TIMERS

/* The barrier functions are available. */
#define _POSIX_BARRIERS	200912L

/* POSIX message queues are not yet supported. */
#undef _POSIX_MESSAGE_PASSING

#endif
