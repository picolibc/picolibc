/* signal.h

  This file is part of Cygwin.

  This software is a copyrighted work licensed under the terms of the
  Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
  details. */

#ifndef _CYGWIN_SIGNAL_H
#define _CYGWIN_SIGNAL_H

#include <bits/wordsize.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
  Define a struct __mcontext, which should be identical in layout to the Win32
  API type CONTEXT with the addition of oldmask and cr2 fields at the end.
*/
#ifdef __x86_64__

struct _uc_fpxreg {
  __uint16_t significand[4];
  __uint16_t exponent;
  __uint16_t padding[3];
};

struct _uc_xmmreg {
  __uint32_t element[4];
};

struct _fpstate
{
  __uint16_t cwd;
  __uint16_t swd;
  __uint16_t ftw;
  __uint16_t fop;
  __uint64_t rip;
  __uint64_t rdp;
  __uint32_t mxcsr;
  __uint32_t mxcr_mask;
  struct _uc_fpxreg st[8];
  struct _uc_xmmreg xmm[16];
  __uint32_t padding[24];
};

struct __attribute__ ((__aligned__ (16))) __mcontext
{
  __uint64_t p1home;
  __uint64_t p2home;
  __uint64_t p3home;
  __uint64_t p4home;
  __uint64_t p5home;
  __uint64_t p6home;
  __uint32_t ctxflags;
  __uint32_t mxcsr;
  __uint16_t cs;
  __uint16_t ds;
  __uint16_t es;
  __uint16_t fs;
  __uint16_t gs;
  __uint16_t ss;
  __uint32_t eflags;
  __uint64_t dr0;
  __uint64_t dr1;
  __uint64_t dr2;
  __uint64_t dr3;
  __uint64_t dr6;
  __uint64_t dr7;
  __uint64_t rax;
  __uint64_t rcx;
  __uint64_t rdx;
  __uint64_t rbx;
  __uint64_t rsp;
  __uint64_t rbp;
  __uint64_t rsi;
  __uint64_t rdi;
  __uint64_t r8;
  __uint64_t r9;
  __uint64_t r10;
  __uint64_t r11;
  __uint64_t r12;
  __uint64_t r13;
  __uint64_t r14;
  __uint64_t r15;
  __uint64_t rip;
  struct _fpstate fpregs;
  __uint64_t vregs[52];
  __uint64_t vcx;
  __uint64_t dbc;
  __uint64_t btr;
  __uint64_t bfr;
  __uint64_t etr;
  __uint64_t efr;
  __uint64_t oldmask;
  __uint64_t cr2;
};

#else /* !x86_64 */

struct _uc_fpreg
{
  __uint16_t significand[4];
  __uint16_t exponent;
};

struct _fpstate
{
  __uint32_t cw;
  __uint32_t sw;
  __uint32_t tag;
  __uint32_t ipoff;
  __uint32_t cssel;
  __uint32_t dataoff;
  __uint32_t datasel;
  struct _uc_fpreg _st[8];
  __uint32_t nxst;
};

struct __mcontext
{
  __uint32_t ctxflags;
  __uint32_t dr0;
  __uint32_t dr1;
  __uint32_t dr2;
  __uint32_t dr3;
  __uint32_t dr6;
  __uint32_t dr7;
  struct _fpstate fpstate;
  __uint32_t gs;
  __uint32_t fs;
  __uint32_t es;
  __uint32_t ds;
  __uint32_t edi;
  __uint32_t esi;
  __uint32_t ebx;
  __uint32_t edx;
  __uint32_t ecx;
  __uint32_t eax;
  __uint32_t ebp;
  __uint32_t eip;
  __uint32_t cs;
  __uint32_t eflags;
  __uint32_t esp;
  __uint32_t ss;
  __uint32_t reserved[128];
  __uint32_t oldmask;
  __uint32_t cr2;
};

#endif /* !x86_64 */

/* Needed for GDB.  It only compiles in the context copy code if this macro is
   defined.  This is not sizeof(CONTEXT) due to historical accidents. */
#ifdef __x86_64__
#define __COPY_CONTEXT_SIZE 816
#else
#define __COPY_CONTEXT_SIZE 204
#endif

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

#if __POSIX_VISIBLE >= 199309

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
    int64_t _si_pipe_unique_id;
    char *_si_str;
  };
};

#define __SI_PAD_SIZE 32
#ifdef __INSIDE_CYGWIN__
# ifndef max
#   define max(a,b) (((a) > (b)) ? (a) : (b))
# endif /*max*/
# define __uint32_size(__x) (max(sizeof (__x) / sizeof (uint32_t), 1))

/* This padding represents the elements of the last struct in siginfo_t,
   aligning the elements to the end to avoid conflicts with other struct
   members. */
# define __SI_CYG_PAD (__SI_PAD_SIZE - __uint32_size (void *))
#endif /*__INSIDE_CYGWIN__*/

typedef struct
{
  int si_signo;				/* signal number */
  int si_code;				/* signal code */
  pid_t si_pid;				/* sender's pid */
  uid_t si_uid;				/* sender's uid */
  int si_errno;				/* errno associated with signal */

  __extension__ union
  {
    __uint32_t __pad[__SI_PAD_SIZE];	/* plan for future growth */
    struct _sigcommune _si_commune;	/* cygwin ipc */
    __extension__ struct
    {
      __extension__ union
      {
	sigval_t si_sigval;		/* signal value */
	sigval_t si_value;		/* signal value */
      };
      __extension__ struct
      {
	timer_t si_tid;			/* timer id */
	unsigned int si_overrun;	/* overrun count */
      };
    };
    /* SIGCHLD */
    __extension__ struct
    {
      int si_status;			/* exit code */
      clock_t si_utime;			/* user time */
      clock_t si_stime;			/* system time */
    };

    void *si_addr;			/* faulting address for core dumping
					   signals */
    /* Cygwin internal fields */
#ifdef __INSIDE_CYGWIN__
    __extension__ struct 
    {
      __uint32_t __pad2[__SI_CYG_PAD];	/* Locate at end of struct */
      void *si_cyg;			/* pointer to block containing
					   cygwin-special info */
    };
#endif /*__INSIDE_CYGWIN__*/
  };
} siginfo_t;

#pragma pack(pop)

#endif /* __POSIX_VISIBLE >= 199309 */

enum
{
  SI_USER = 0,				/* sent by kill, raise, pthread_kill */
  SI_ASYNCIO = 2,			/* sent by AIO completion (currently
					   unimplemented) */
  SI_MESGQ,				/* sent by real time mesq state change
					   (currently unimplemented) */
  SI_TIMER,				/* sent by timer expiration */
  SI_QUEUE,				/* sent by sigqueue */
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

#define SIGEV_SIGNAL SIGEV_SIGNAL
#define SIGEV_NONE   SIGEV_NONE
#define SIGEV_THREAD SIGEV_THREAD

typedef void (*_sig_func_ptr)(int);

#if __POSIX_VISIBLE

struct sigaction
{
  __extension__ union
  {
    _sig_func_ptr sa_handler;  		/* SIG_DFL, SIG_IGN, or pointer to a function */
#if __POSIX_VISIBLE >= 199309
    void  (*sa_sigaction) ( int, siginfo_t *, void * );
#endif
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
#define SA_ONSTACK   0x20000000		/* Call signal handler on alternate
					   signal stack provided by
					   sigaltstack(2). */
#define SA_NODEFER   0x40000000		/* Don't automatically block the signal
					   when its handler is being executed  */
#define SA_RESETHAND 0x80000000		/* Reset to SIG_DFL on entry to handler */
#define SA_ONESHOT   SA_RESETHAND	/* Historical linux name */
#define SA_NOMASK    SA_NODEFER		/* Historical linux name */

/* Used internally by cygwin.  Included here to group everything in one place.
   Do not use.  */
#define _SA_INTERNAL_MASK 0xf000	/* bits in this range are internal */

#endif /* __POSIX_VISIBLE */

#if __BSD_VISIBLE || __XSI_VISIBLE >= 4 || __POSIX_VISIBLE >= 200809

#undef	MINSIGSTKSZ
#define	MINSIGSTKSZ	 8192
#undef	SIGSTKSZ
#define	SIGSTKSZ	32768

#endif /* __BSD_VISIBLE || __XSI_VISIBLE >= 4 || __POSIX_VISIBLE >= 200809 */

#define	SIGHUP	1	/* hangup */
#define	SIGINT	2	/* interrupt */
#define	SIGQUIT	3	/* quit */
#define	SIGILL	4	/* illegal instruction (not reset when caught) */
#define	SIGTRAP	5	/* trace trap (not reset when caught) */
#define	SIGABRT 6	/* used by abort */
#define	SIGIOT	SIGABRT	/* synonym for SIGABRT on most systems */
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
#define	SIGPWR  SIGLOST	/* power failure */
#define	SIGUSR1 30	/* user defined signal 1 */
#define	SIGUSR2 31	/* user defined signal 2 */

#if __WORDSIZE == 64
#define NSIG	65      /* signal 0 implied */
#else
#define NSIG	33      /* signal 0 implied */
#endif

/* Real-Time signals per SUSv3.  RT_SIGMAX is defined as 8 in limits.h */
#define SIGRTMIN 32
#define SIGRTMAX (NSIG - 1)

#define SIG_HOLD ((_sig_func_ptr)2)	/* Signal in signal mask */

#if __POSIX_VISIBLE >= 200809
void psiginfo (const siginfo_t *, const char *);
#endif
#if __POSIX_VISIBLE
int sigwait (const sigset_t *, int *);
#endif
#if __POSIX_VISIBLE >= 199309
int sigwaitinfo (const sigset_t *, siginfo_t *);
#endif
#if __XSI_VISIBLE >= 4
int sighold (int);
int sigignore (int);
int sigrelse (int);
_sig_func_ptr sigset (int, _sig_func_ptr);
#endif

#if __POSIX_VISIBLE >= 199309
int sigqueue(pid_t, int, const union sigval);
#endif
#if __BSD_VISIBLE || __XSI_VISIBLE >= 4 || __POSIX_VISIBLE >= 200809
int siginterrupt (int, int);
#endif
#ifdef __INSIDE_CYGWIN__
extern const char *sys_sigabbrev[];
extern const char *sys_siglist[];
#elif __BSD_VISIBLE
extern const char __declspec(dllimport) *sys_sigabbrev[];
extern const char __declspec(dllimport) *sys_siglist[];
#endif

#ifdef __cplusplus
}
#endif
#endif /*_CYGWIN_SIGNAL_H*/
