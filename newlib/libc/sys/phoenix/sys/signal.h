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

#ifndef _SYS_SIGNAL_H
#define _SYS_SIGNAL_H

#include <phoenix/signal.h>
#include <sys/types.h>

typedef void (*_sig_func_ptr)(int);
typedef _sig_func_ptr		__sighandler_t;
typedef _sig_func_ptr 		sighandler_t;
typedef unsigned long		sigset_t;

#define SIGEV_NONE		1		/* No asynchronous notification shall be delivered when the event of interest occurs. */
#define SIGEV_SIGNAL	2		/* A queued signal, with an application defined value, shall be delivered when the event of interest occurs. */
#define SIGEV_THREAD	3		/* A notification function shall be called to perform notification. */

union sigval {
	int sival_int;				/* Integer signal value */
	void *sival_ptr;			/* Pointer signal value */
};

struct sigevent {
	int sigev_notify;				/* Notification type */
	int sigev_signo;				/* Signal number */
	union sigval sigev_value;		/* Signal value */
#if defined(_POSIX_THREADS)
	void (*sigev_notify_function)(union sigval);	/* Notification function */
	pthread_attr_t  *sigev_notify_attributes;		/* Notification Attributes */
#endif
};

#define SI_USER			1		/* Sent by a user. kill(), abort(), etc */
#define SI_QUEUE		2		/* Sent by sigqueue() */
#define SI_TIMER		3		/* Sent by expiration of a timer_settime() timer */
#define SI_ASYNCIO		4		/* Indicates completion of asycnhronous IO */
#define SI_MESGQ		5		/* Indicates arrival of a message at an empty queue */

typedef struct {
	int si_signo;				/* Signal number */
	int si_code;				/* Cause of the signal */
	union sigval si_value;		/* Signal value */
} siginfo_t;

#define SA_NOCLDSTOP	1		/* Do not generate SIGCHLD when children stop */
#define SA_SIGINFO		2		/* Invoke the signal catching function with three arguments instead of one. */
#define SA_RESTART		4 

struct sigaction {
	int sa_flags;				/* Special flags to affect behavior of signal */
	sigset_t sa_mask;			/* Additional set of signals to be blocked during execution of signal-catching function. */
	union {
		_sig_func_ptr _handler;	/* SIG_DFL, SIG_IGN, or pointer to a function */
		void (*_sigaction)(int, siginfo_t *, void *);
	} _signal_handlers;
};

#define sa_handler		_signal_handlers._handler
#define sa_sigaction	_signal_handlers._sigaction

#define SIG_SETMASK		0		/* Set mask with sigprocmask() */
#define SIG_BLOCK		1		/* Set of signals to block */
#define SIG_UNBLOCK		2		/* Set of signals to, well, unblock */

int kill(pid_t pid, int sig);
int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
int sigaddset(sigset_t *set, int signum);
int sigdelset(sigset_t *set, int signum);
int sigismember(const sigset_t *set, int signum);
int sigfillset(sigset_t *set);
int sigemptyset(sigset_t *set);
int sigpending(sigset_t *set);
int sigsuspend(const sigset_t *mask);
int sigpause(int sigmask);
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
int sigblock(int mask);
int sigmask(int signum);
int sigsetmask(int mask);

#endif
