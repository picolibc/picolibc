/* sys/signal.h */

#ifndef _SYS_SIGNAL_H
#define _SYS_SIGNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "_ansi.h"

#ifndef __STRICT_ANSI__
typedef unsigned long sigset_t;
struct sigaction 
{
	void (*sa_handler)(int);
	sigset_t sa_mask;
	int sa_flags;
};
#define SA_NOCLDSTOP 1	/* only value supported now for sa_flags */
#define SIG_SETMASK 0	/* set mask with sigprocmask() */
#define SIG_BLOCK 1	/* set of signals to block */
#define SIG_UNBLOCK 2	/* set of signals to, well, unblock */

/* These depend upon the type of sigset_t, which right now 
   is always a long.. They're in the POSIX namespace, but
   are not ANSI. */
#define sigaddset(what,sig) (*(what) |= (1<<(sig)))
#define sigemptyset(what)   (*(what) = 0)

int sigprocmask (int __how, const sigset_t *__a, sigset_t *__b);

/* protos for functions found in winsup sources */
#if defined(__CYGWIN32__)
#undef sigaddset
#undef sigemptyset
/* The first argument to kill should be pid_t.  Right now
   <sys/types.h> always defines pid_t to be int.  If that ever
   changes, then we will need to do something else, perhaps along the
   lines of <machine/types.h>.  */
int _EXFUN(kill, (int, int));
int _EXFUN(sigaction, (int, const struct sigaction *, struct sigaction *));
int _EXFUN(sigaddset, (sigset_t *, const int));
int _EXFUN(sigdelset, (sigset_t *, const int));
int _EXFUN(sigismember, (const sigset_t *, int));
int _EXFUN(sigfillset, (sigset_t *));
int _EXFUN(sigemptyset, (sigset_t *));
int _EXFUN(sigpending, (sigset_t *));
int _EXFUN(sigsuspend, (const sigset_t *));
int _EXFUN(sigpause, (int));
#endif

#endif /* __STRICT_ANSI__ */

#if defined(___AM29K__)
/* These all need to be defined for ANSI C, but I don't think they are
   meaningful.  */
#define SIGABRT 1
#define SIGFPE 1
#define SIGILL 1
#define SIGINT 1
#define SIGSEGV 1
#define SIGTERM 1
/* These need to be defined for POSIX, and some others do too.  */
#define SIGHUP 1
#define SIGQUIT 1
#define NSIG 2
#elif defined(__GO32__)
#define SIGINT  1
#define SIGKILL 2
#define SIGPIPE 3
#define SIGFPE  4
#define SIGHUP  5
#define SIGTERM 6
#define SIGSEGV 7
#define SIGTSTP 8
#define SIGQUIT 9
#define SIGTRAP 10
#define SIGILL  11
#define SIGEMT  12
#define SIGALRM 13
#define SIGBUS  14
#define SIGLOST 15
#define SIGSTOP 16
#define SIGABRT 17
#define SIGUSR1	18
#define SIGUSR2	19
#define NSIG    20
#elif defined(__CYGWIN32__)	/* BSD signals symantics */
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
#define NSIG	32      /* signal 0 implied */
#else
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

#if defined(__svr4__)
/* svr4 specifics. different signals above 15, and sigaction. */
#define	SIGUSR1	16
#define SIGUSR2	17
#define SIGCLD	18
#define	SIGPWR	19
#define SIGWINCH 20
#define	SIGPOLL	22	/* 20 for x.out binaries!!!! */
#define	SIGSTOP	23	/* sendable stop signal not from tty */
#define	SIGTSTP	24	/* stop signal from tty */
#define	SIGCONT	25	/* continue a stopped process */
#define	SIGTTIN	26	/* to readers pgrp upon background tty read */
#define	SIGTTOU	27	/* like TTIN for output if (tp->t_local&LTOSTOP) */
#define NSIG	28	
#else
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
#define NSIG	32      /* signal 0 implied */
#endif
#endif

#ifdef __cplusplus
}
#endif
#endif /* _SYS_SIGNAL_H */
