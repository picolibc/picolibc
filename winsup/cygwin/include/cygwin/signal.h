/* signal.h

  Copyright 2004, 2005, 2006 Red Hat, Inc.

  This file is part of Cygwin.

  This software is a copyrighted work licensed under the terms of the
  Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
  details. */

#ifndef _CYGWIN_SIGNAL_H
#define _CYGWIN_SIGNAL_H

#ifdef __cplusplus
extern "C" {
#endif
struct _fpstate
{
  unsigned long cw;
  unsigned long sw;
  unsigned long tag;
  unsigned long ipoff;
  unsigned long cssel;
  unsigned long dataoff;
  unsigned long datasel;
  unsigned char _st[80];
  unsigned long nxst;
};

struct ucontext
{
  unsigned long cr2;
  unsigned long dr0;
  unsigned long dr1;
  unsigned long dr2;
  unsigned long dr3;
  unsigned long dr6;
  unsigned long dr7;
  struct _fpstate fpstate;
  unsigned long gs;
  unsigned long fs;
  unsigned long es;
  unsigned long ds;
  unsigned long edi;
  unsigned long esi;
  unsigned long ebx;
  unsigned long edx;
  unsigned long ecx;
  unsigned long eax;
  unsigned long ebp;
  unsigned long eip;
  unsigned long cs;
  unsigned long eflags;
  unsigned long esp;
  unsigned long ss;
  unsigned char _internal;
  unsigned long oldmask;
};

#define __COPY_CONTEXT_SIZE ((unsigned) &((struct ucontext *) 0)->_internal)

typedef union sigval
{
  int sival_int;			/* integer signal value */
  void  *sival_ptr;			/* pointer signal value */
} sigval_t;

typedef struct sigevent
{
  sigval_t sigev_value;			/* signal value */
  int sigev_signo;			/* signal number */
  int sigev_notify;			/* notification type */
  void (*sigev_notify_function) (sigval_t); /* notification function */
  pthread_attr_t *sigev_notify_attributes; /* notification attributes */
} sigevent_t;

#pragma pack(push,4)
struct _sigcommune
{
  __uint32_t _si_code;
  void *_si_read_handle;
  void *_si_write_handle;
  void *_si_process_handle;
  __extension__ union
  {
    int _si_fd;
    void *_si_pipe_fhandler;
    char *_si_str;
  };
};

typedef struct
{
  int si_signo;				/* signal number */
  int si_code;				/* signal code */
  pid_t si_pid;				/* sender's pid */
  uid_t si_uid;				/* sender's uid */
  int si_errno;				/* errno associated with signal */

  __extension__ union
  {
    __uint32_t __pad[32];		/* plan for future growth */
    struct _sigcommune _si_commune;	/* cygwin ipc */
    union
    {
      /* timers */
      struct
      {
	union
	{
	  struct
	  {
	    timer_t si_tid;		/* timer id */
	    unsigned int si_overrun;	/* overrun count */
	  };
	  sigval_t si_sigval;		/* signal value */
	  sigval_t si_value;		/* signal value */
	};
      };
    };

    /* SIGCHLD */
    __extension__ struct
    {
      int si_status;			/* exit code */
      clock_t si_utime;			/* user time */
      clock_t si_stime;			/* system time */
    };

    /* core dumping signals */
    void *si_addr;			/* faulting address */
  };
} siginfo_t;
#pragma pack(pop)

enum
{
  SI_USER = 0,				/* sent by kill, raise, pthread_kill */
  SI_ASYNCIO = 2,			/* sent by AIO completion (currently
					   unimplemented) */
  SI_MESGQ,				/* sent by real time mesq state change
					   (currently unimplemented) */
  SI_TIMER,				/* sent by timer expiration */
  SI_QUEUE,				/* sent by sigqueue (currently
					   unimplemented) */
  SI_KERNEL,				/* sent by system */

  ILL_ILLOPC,				/* illegal opcode */
  ILL_ILLOPN,				/* illegal operand */
  ILL_ILLADR,				/* illegal addressing mode */
  ILL_ILLTRP,				/* illegal trap*/
  ILL_PRVOPC,				/* privileged opcode */
  ILL_PRVREG,				/* privileged register */
  ILL_COPROC,				/* coprocessor error */
  ILL_BADSTK,				/* internal stack error */

  FPE_INTDIV,				/* integer divide by zero */
  FPE_INTOVF,				/* integer overflow */
  FPE_FLTDIV,				/* floating point divide by zero */
  FPE_FLTOVF,				/* floating point overflow */
  FPE_FLTUND,				/* floating point underflow */
  FPE_FLTRES,				/* floating point inexact result */
  FPE_FLTINV,				/* floating point invalid operation */
  FPE_FLTSUB,				/* subscript out of range */

  SEGV_MAPERR,				/* address not mapped to object */
  SEGV_ACCERR,				/* invalid permissions for mapped object */

  BUS_ADRALN,				/* invalid address alignment.  */
  BUS_ADRERR,				/* non-existant physical address.  */
  BUS_OBJERR,				/* object specific hardware error.  */

  CLD_EXITED,				/* child has exited */
  CLD_KILLED,				/* child was killed */
  CLD_DUMPED,				/* child terminated abnormally */
  CLD_TRAPPED,				/* traced child has trapped */
  CLD_STOPPED,				/* child has stopped */
  CLD_CONTINUED				/* stopped child has continued */
};

enum
{
  SIGEV_SIGNAL = 0,			/* a queued signal, with an application
					   defined value, is generated when the
					   event of interest occurs */
  SIGEV_NONE,				/* no asynchronous notification is
					   delivered when the event of interest
					   occurs */
  SIGEV_THREAD				/* a notification function is called to
					   perform notification */
};

typedef void (*_sig_func_ptr)(int);

struct sigaction
{
  __extension__ union
  {
    _sig_func_ptr sa_handler;  		/* SIG_DFL, SIG_IGN, or pointer to a function */
    void  (*sa_sigaction) ( int, siginfo_t *, void * );
  };
  sigset_t sa_mask;
  int sa_flags;
};

#define SA_NOCLDSTOP 1   		/* Do not generate SIGCHLD when children
					   stop */
#define SA_SIGINFO   2   		/* Invoke the signal catching function
					   with three arguments instead of one
					 */
#define SA_RESTART   0x10000000 	/* Restart syscall on signal return */
#define SA_NODEFER   0x40000000		/* Don't automatically block the signal
					   when its handler is being executed  */
#define SA_RESETHAND 0x80000000		/* Reset to SIG_DFL on entry to handler */
#define SA_ONESHOT   SA_RESETHAND	/* Historical linux name */
#define SA_NOMASK    SA_NODEFER		/* Historical linux name */

/* Used internally by cygwin.  Included here to group everything in one place.
   Do not use.  */
#define _SA_INTERNAL_MASK 0xf000	/* bits in this range are internal */

#define	SIGHUP	1	/* hangup */
#define	SIGINT	2	/* interrupt */
#define	SIGQUIT	3	/* quit */
#define	SIGILL	4	/* illegal instruction (not reset when caught) */
#define	SIGTRAP	5	/* trace trap (not reset when caught) */
#define	SIGABRT 6	/* used by abort */
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

/* Real-Time signals per SUSv3.  RT_SIGMAX is defined as 8 in limits.h */
#define SIGRTMIN 32
#define SIGRTMAX ((SIGRTMIN) + 0)
#define NSIG	33      /* signal 0 implied */

#define SIG_HOLD ((_sig_func_ptr)2)	/* Signal in signal mask */

int sigwait (const sigset_t *, int *);
int sigwaitinfo (const sigset_t *, siginfo_t *);
int sighold (int);
int sigignore (int);
int sigrelse (int);
_sig_func_ptr sigset (int, _sig_func_ptr);

int sigqueue(pid_t, int, const union sigval);
int siginterrupt (int, int);
#ifdef __cplusplus
}
#endif
#endif /*_CYGWIN_SIGNAL_H*/
