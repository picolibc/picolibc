/*
Copyright (c) 1991, 1993
The Regents of the University of California.  All rights reserved.
c) UNIX System Laboratories, Inc.
All or some portions of this file are derived from material licensed
to the University of California by American Telephone and Telegraph
Co. or Unix System Laboratories, Inc. and are reproduced herein with
the permission of UNIX System Laboratories, Inc.

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
#ifndef _SIGNAL_H_
#define _SIGNAL_H_

#include <sys/cdefs.h>
#define __need_size_t
#include <stddef.h>
#include <sys/_types.h>
#include <sys/_sigset.h>
#include <sys/_timespec.h>

_BEGIN_STD_C

typedef int	sig_atomic_t;		/* Atomic entity type (ANSI) */

typedef void (*_sig_func_ptr)(int);

#define SIG_DFL ((_sig_func_ptr)0)	/* Default action */
#define SIG_IGN ((_sig_func_ptr)1)	/* Ignore action */
#define SIG_ERR ((_sig_func_ptr)-1)	/* Error return */

#if __POSIX_VISIBLE

#ifndef _UID_T_DECLARED
typedef	__uid_t		uid_t;		/* user id */
#define	_UID_T_DECLARED
#endif

#if !defined(_SIGSET_T_DECLARED)
#define	_SIGSET_T_DECLARED
typedef	__sigset_t	sigset_t;
#endif

union sigval {
  int    sival_int;    /* Integer signal value */
  void  *sival_ptr;    /* Pointer signal value */
};

/* Signal Actions, P1003.1b-1993, p. 64 */
/* si_code values, p. 66 */

#define SI_USER    1    /* Sent by a user. kill(), abort(), etc */
#define SI_QUEUE   2    /* Sent by sigqueue() */
#define SI_TIMER   3    /* Sent by expiration of a timer_settime() timer */
#define SI_ASYNCIO 4    /* Indicates completion of asycnhronous IO */
#define SI_MESGQ   5    /* Indicates arrival of a message at an empty queue */

typedef struct {
        int          si_signo;    /* Signal number */
        int          si_code;     /* Cause of the signal */
        int          si_errno;    /* If non-zero, the errno */
        __pid_t      si_pid;      /* Sending process ID */
        uid_t        si_uid;      /* Real UID of sending process */
        void         *si_addr;    /* Address of faulting instruction */
        int          si_status;   /* Exit value or signal */
        union sigval si_value;    /* Signal value */
} siginfo_t;

/*
 * Possible values for sa_flags in sigaction below.
 */

#define SA_NOCLDSTOP    (1 << 0)
#define SA_ONSTACK      (1 << 1)
#define SA_RESETHAND    (1 << 2)
#define SA_RESTART      (1 << 3)
#define SA_SIGINFO      (1 << 4)
#define SA_NOCLDWAIT    (1 << 5)
#define SA_NODEFER      (1 << 6)

struct sigaction {
        union {
                void    (*sa_handler)(int);
                void    (*sa_sigaction)(int, siginfo_t *, void *);
        };
	sigset_t  sa_mask;
	int       sa_flags;
};

/*
 * Possible values for ss_flags in stack_t below.
 */
#define	SS_ONSTACK	0x1
#define	SS_DISABLE	0x2

/*
 * Structure used in sigaltstack call.
 */
typedef struct sigaltstack {
  void     *ss_sp;    /* Stack base or pointer.  */
  int       ss_flags; /* Flags.  */
  size_t    ss_size;  /* Stack size.  */
} stack_t;

#define SIG_SETMASK 0	/* set mask with sigprocmask() */
#define SIG_BLOCK 1	/* set of signals to block */
#define SIG_UNBLOCK 2	/* set of signals to unblock */

#endif /* __POSIX_VISIBLE */

#if defined(_POSIX_REALTIME_SIGNALS) || __POSIX_VISIBLE >= 199309

/* sigev_notify values
   NOTE: P1003.1c/D10, p. 34 adds SIGEV_THREAD.  */

#define SIGEV_NONE   1  /* No asynchronous notification shall be delivered */
                        /*   when the event of interest occurs. */
#define SIGEV_SIGNAL 2  /* A queued signal, with an application defined */
                        /*  value, shall be delivered when the event of */
                        /*  interest occurs. */
#define SIGEV_THREAD 3  /* A notification function shall be called to */
                        /*   perform notification. */

/*  Signal Generation and Delivery, P1003.1b-1993, p. 63
    NOTE: P1003.1c/D10, p. 34 adds sigev_notify_function and
          sigev_notify_attributes to the sigevent structure.  */

struct sigevent {
  int              sigev_notify;               /* Notification type */
  int              sigev_signo;                /* Signal number */
  union sigval     sigev_value;                /* Signal value */
  void           (*sigev_notify_function)( union sigval );
                                               /* Notification function */
  void            *sigev_notify_attributes;    /* Notification Attributes */
};
#endif /* defined(_POSIX_REALTIME_SIGNALS) || __POSIX_VISIBLE >= 199309 */


#if __BSD_VISIBLE || __XSI_VISIBLE >= 4 || __POSIX_VISIBLE >= 200809

/*
 * Minimum and default signal stack constants. Allow for target overrides
 * from <sys/features.h>.
 */
#ifndef	MINSIGSTKSZ
#define	MINSIGSTKSZ	2048
#endif
#ifndef	SIGSTKSZ
#define	SIGSTKSZ	8192
#endif

#endif

#define	SIGHUP	1	/* hangup */
#define	SIGINT	2	/* interrupt */
#define	SIGQUIT	3	/* quit */
#define	SIGILL	4	/* illegal instruction (not reset when caught) */
#define	SIGTRAP	5	/* trace trap (not reset when caught) */
#define	SIGIOT	6	/* IOT instruction */
#define	SIGABRT 6	/* used by abort, replace SIGIOT in the future */
#define	SIGEMT	7	/* EMT instruction */
#define	SIGFPE	8	/* floating point exception */
#define	SIGKILL	9	/* kill (cannot be caught or ignored) */
#define	SIGBUS	10	/* bus error */
#define	SIGSEGV	11	/* segmentation violation */
#define	SIGSYS	12	/* bad argument to system call */
#define	SIGPIPE	13	/* write on a pipe with no one to read it */
#define	SIGALRM	14	/* alarm clock */
#define	SIGTERM	15	/* software termination signal from kill */
#define	SIGURG	16	/* urgent condition on IO channel */
#define	SIGSTOP	17	/* sendable stop signal not from tty */
#define	SIGTSTP	18	/* stop signal from tty */
#define	SIGCONT	19	/* continue a stopped process */
#define	SIGCHLD	20	/* to parent on child stop or exit */
#define	SIGCLD	20	/* System V name for SIGCHLD */
#define	SIGTTIN	21	/* to readers pgrp upon background tty read */
#define	SIGTTOU	22	/* like TTIN for output if (tp->t_local&LTOSTOP) */
#define	SIGIO	23	/* input/output possible signal */
#define	SIGPOLL	SIGIO	/* System V name for SIGIO */
#define	SIGXCPU	24	/* exceeded CPU time limit */
#define	SIGXFSZ	25	/* exceeded file size limit */
#define	SIGVTALRM 26	/* virtual time alarm */
#define	SIGPROF	27	/* profiling time alarm */
#define	SIGWINCH 28	/* window changed */
#define	SIGLOST 29	/* resource lost (eg, record-lock lost) */
#define	SIGUSR1 30	/* user defined signal 1 */
#define	SIGUSR2 31	/* user defined signal 2 */
#define _NSIG   32

/* Using __MISC_VISIBLE until POSIX Issue 8 is officially released */
#if __MISC_VISIBLE
#if __SIZEOF_INT__ >= 4
#define SIG2STR_MAX 17	/* (sizeof("RTMAX+") + sizeof("4294967295") - 1) */
#else
#define SIG2STR_MAX 12	/* (sizeof("RTMAX+") + sizeof("65535") - 1) */
#endif
#endif

#if __BSD_VISIBLE
typedef _sig_func_ptr sig_t;		/* BSD naming */
#endif

#if __GNU_VISIBLE
typedef _sig_func_ptr sighandler_t;	/* glibc naming */
#endif

#if __POSIX_VISIBLE
int     kill(__pid_t pid, int sig);
#endif
#if __XSI_VISIBLE >= 500
int     killpg(__pid_t pid, int sig);
#endif
#if __POSIX_VISIBLE >= 200809L
void    psiginfo(const siginfo_t *, const char *);
#endif
#if __BSD_VISIBLE || __SVID_VISIBLE
void	psignal (int, const char *);
#endif
int	raise (int);
#if __MISC_VISIBLE
int     sig2str(int, char *);
#endif
#if __POSIX_VISIBLE
int     sigaction(int, const struct sigaction *__restrict, struct sigaction *__restrict);
int     sigaddset (sigset_t *, const int);
#define sigaddset(what,sig) (*(what) |= (1<<(sig)), 0)
#endif
#if __BSD_VISIBLE || __XSI_VISIBLE >= 4 || __POSIX_VISIBLE >= 200809
int     sigaltstack (const stack_t *__restrict, stack_t *__restrict);
#endif
#if __POSIX_VISIBLE
int     sigdelset (sigset_t *, const int);
#define sigdelset(what,sig) (*(what) &= ~((sigset_t)1<<(sig)), 0)
int     sigemptyset (sigset_t *);
#define sigemptyset(what)   (*(what) = (sigset_t) 0, 0)
int     sigfillset (sigset_t *);
#define sigfillset(what)    (*(what) = ~((sigset_t)0), 0)
int     sigismember (const sigset_t *, int);
#define sigismember(what,sig) (((*(what)) & ((sigset_t) 1<<(sig))) != 0)
#endif
_sig_func_ptr signal (int, _sig_func_ptr);
#if __POSIX_VISIBLE
int     sigpending (sigset_t *);
int     sigprocmask (int, const sigset_t *, sigset_t *);
#endif
#if __POSIX_VISIBLE >= 199309L
int     sigqueue (__pid_t, int, const union sigval);
#endif
#if __POSIX_VISIBLE
int     sigsuspend (const sigset_t *);
#endif
#if __POSIX_VISIBLE >= 199309L
int     sigtimedwait (const sigset_t *, siginfo_t *, const struct timespec *);
#endif
#if __POSIX_VISIBLE >= 199506L
int     sigwait (const sigset_t *, int *);
#endif
#if __POSIX_VISIBLE >= 199309L
int     sigwaitinfo (const sigset_t *, siginfo_t *);
#endif
#if __MISC_VISIBLE
int str2sig(const char *__restrict, int *__restrict);
#endif

_END_STD_C

#endif /* _SIGNAL_H_ */
