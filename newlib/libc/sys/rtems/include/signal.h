/*
 *  $Id$
 */

#ifndef __POSIX_SIGNALS_h
#define __POSIX_SIGNALS_h

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/features.h>
#include "_ansi.h"

#include <sys/time.h>
#include <sys/siginfo.h>
#include <reent.h>        /* only for reentrant signal() and raise() */

/*
 *  7.7 Signal handling <signal.h>, ANSI C Standard.
 */

typedef int sig_atomic_t;

/*
 *  ANSI C Signal Handling Functions
 */

typedef void (*_sig_func_ptr) ();
 
_sig_func_ptr _EXFUN(_signal_r, (struct _reent *, int, _sig_func_ptr));
int _EXFUN(_raise_r, (struct _reent *, int));
 
#ifndef _REENT_ONLY
_sig_func_ptr _EXFUN(signal, (int, _sig_func_ptr));
int _EXFUN(raise, (int));
#endif

/*
 *  3.3.2 Send a Signal to a Process, P1003.1b-1993, p. 68
 *
 *  NOTE: Behavior of kill() depends on _POSIX_SAVED_IDS.
 */

int _EXFUN(kill, (pid_t pid, int sig));

/*
 *  3.3.3 Manipulate Signal Sets, P1003.1b-1993, p. 69
 */

int _EXFUN(sigemptyset, (sigset_t *set));
int _EXFUN(sigfillset,  (sigset_t *set));
int _EXFUN(sigaddset,   (sigset_t *set, int signo));
int _EXFUN(sigdelset,   (sigset_t *set, int signo));
int _EXFUN(sigismember, (const sigset_t *set, int signo));

/*
 *  3.3.4 Examine and Change Signal Action, P1003.1b-1993, p. 70
 */

int _EXFUN(sigaction,
           (int sig, const struct sigaction *act, struct sigaction *oact)
);

/*
 *  3.3.5 Examine and Change Blocked Signals, P1003.1b-1993, p. 73
 *
 *  NOTE: P1003.1c/D10, p. 37 adds pthread_sigmask().
 */

/* values for how */

#define SIG_BLOCK   1 /* resulting_set = (current_set | set)  */
#define SIG_UNBLOCK 2 /* resulting_set = (current_set & ~set)  */ 
#define SIG_SETMASK 3 /* resulting_set = set */

int _EXFUN(sigprocmask, (int how, const sigset_t *set, sigset_t *oset));

#if defined(_POSIX_THREADS)
int _EXFUN(pthread_sigmask, (int how, const sigset_t *set, sigset_t *oset));
#endif

/*
 *  3.3.6 Examine Pending Signals, P1003.1b-1993, p. 75
 */

int _EXFUN(sigpending, (sigset_t *set));

/*
 *  3.3.7 Wait for a Signal, P1003.1b-1993, p. 75
 */

int _EXFUN(sigsuspend, (const sigset_t *sigmask));

#if defined(_POSIX_REALTIME_SIGNALS)

/*
 *  3.3.8 Synchronously Accept a Signal, P1003.1b-1993, p. 76
 *
 *  NOTE: P1003.1c/D10, p. 39 adds sigwait().
 */

int _EXFUN(sigwaitinfo,  (const sigset_t *set, siginfo_t *info));
int _EXFUN(sigtimedwait,
  (const sigset_t *set, siginfo_t *info, const struct timespec  *timeout)
);
int _EXFUN(sigwait, (const sigset_t *set, int *sig));

/*
 *  3.3.9 Queue a Signal to a Process, P1003.1b-1993, p. 78
 */

int _EXFUN(sigqueue, (pid_t pid, int signo, const union sigval value));

#endif

/*
 *  3.3.10 Send a Signal to a Thread, P1003.1c/D10, p. 43
 */

#if defined(_POSIX_THREADS)
int _EXFUN(pthread_kill, (pthread_t thread, int sig));
#endif

/*
 *  3.4.1 Schedule Alarm, P1003.1b-1993, p. 79
 */

unsigned int _EXFUN(alarm, (unsigned int seconds));

/*
 *  3.4.2 Suspend Process Execution, P1003.1b-1993, p. 80
 */

int _EXFUN(pause, (void));

#ifdef __cplusplus
}
#endif

#endif
/* end of include file */
