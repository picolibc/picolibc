/*
 *  $Id$
 */

#ifndef __POSIX_SYS_SIGNAL_INFORMATION_h
#define __POSIX_SYS_SIGNAL_INFORMATION_h

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_POSIX_THREADS)
#include <sys/types.h>
#endif

/*
 *  3.3  Signal Concepts, P1003.1b-1993, p. 60
 */
 
typedef __uint32_t sigset_t;
 
#define SIG_DFL   ((void (*)())0)  /* Request for default signal handling */
#define SIG_IGN   ((void (*)())1)  /* Request that signal be ignored */

#define SIG_ERR   ((void (*)())-1) /* Returned by signal() on error */
 
/*
 *  Required Signals.
 *
 *   The default action is in parentheses and is one of the
 *   following actions:
 *
 *     (1) abnormal termination of process
 *     (2) ignore the signal
 *     (3) stop the process
 *     (4) continue the process if it is currently stopped, otherwise
 *         ignore the signal
 */
 
#define SIGHUP   1  /* (1) hangup detected on controlling terminal */
#define SIGINT   2  /* (1) interactive attention signal */
#define SIGQUIT  3  /* (1) interactive termination signal */
#define SIGILL   4  /* (1) illegal instruction */
#define SIGTRAP  5  /* (1) trace trap (not reset) */
#define SIGIOT   6  /* (1) IOT instruction */
#define SIGABRT  6  /* (1) abnormal terminal signal */
#define SIGEMT   7  /* (1) EMT instruction */
#define SIGFPE   8  /* (1) erroneous arithmetic operation */
#define SIGKILL  9  /* (1) termination signal */
#define SIGBUS  10  /* (1) bus error */
#define SIGSEGV 11  /* (1) invalid memory reference */
#define SIGSYS  12  /* (1) bad argument to system call */
#define SIGPIPE 13  /* (1) write on pipe with no readers */
#define SIGALRM 14  /* (1) timeout signal, such as initiated by alarm() */
#define SIGTERM 15  /* (1) termination signal */
#define SIGUSR1 16  /* (1) reserved as application defined signal 1 */
#define SIGUSR2 17  /* (1) reserved as application defined signal 2 */
 
#define __SIGFIRSTNOTRT SIGHUP
#define __SIGLASTNOTRT  SIGUSR2
 
/*
 *  RTEMS does not support job control, hence no Job Control Signals are
 *  defined per P1003.1b-1993, p. 60-61.
 */

/*
 *  RTEMS does not support memory protection, hence no Memory Protection
 *  Signals are defined per P1003.1b-1993, p. 60-61.
 */
 
/*
 *  Real-Time Signals Range, P1003.1b-1993, p. 61
 *
 *  NOTE: This should be at least RTSIG_MAX (which is a minimum of 8) signals.
 */
 
#define SIGRTMIN 18
#define SIGRTMAX 32
 
/* sigev_notify values
 *
 *  NOTE: P1003.1c/D10, p. 34 adds SIGEV_THREAD.
 */
 
#define SIGEV_NONE   1  /* No asynchronous notification shall be delivered */
                        /*   when the event of interest occurs. */
#define SIGEV_SIGNAL 2  /* A queued signal, with an application defined */
                        /*  value, shall be delivered when the event of */
                        /*  interest occurs. */
#define SIGEV_THREAD 3  /* A notification function shall be called to */
                        /*   perform notification. */
/*
 *  3.3.1.2 Signal Generation and Delivery, P1003.1b-1993, p. 63
 *
 *  NOTE: P1003.1c/D10, p. 34 adds sigev_notify_function and 
 *        sigev_notify_attributes to the sigevent structure. 
 */

union sigval {
  int    sival_int;    /* Integer signal value */
  void  *sival_ptr;    /* Pointer signal value */
};

struct sigevent {
  int              sigev_notify;               /* Notification type */
  int              sigev_signo;                /* Signal number */
  union sigval     sigev_value;                /* Signal value */

#if defined(_POSIX_THREADS)
  void           (*sigev_notify_function)( union sigval ); 
                                               /* Notification function */
  pthread_attr_t  *sigev_notify_attributes;    /* Notification Attributes */
#endif
};

/*
 *  3.3.1.3 Signal Actions, P1003.1b-1993, p. 64
 */
 
/* si_code values, p. 66 */
 
#define SI_USER    1    /* Sent by a user. kill(), abort(), etc */
#define SI_QUEUE   2    /* Sent by sigqueue() */
#define SI_TIMER   3    /* Sent by expiration of a timer_settime() timer */
#define SI_ASYNCIO 4    /* Indicates completion of asycnhronous IO */
#define SI_MESGQ   5    /* Indicates arrival of a message at an empty queue */
 
typedef struct {
  int          si_signo;    /* Signal number */
  int          si_code;     /* Cause of the signal */
  union sigval si_value;    /* Signal value */
} siginfo_t;
 
/*
 *  3.3.4 Examine and Change Signal Action, P1003.1b-1993, p. 70
 */
 
/* sa_flags values */
 
#define SA_NOCLDSTOP 1   /* Do not generate SIGCHLD when children stop */
#define SA_SIGINFO   2   /* Invoke the signal catching function with */
                         /*   three arguments instead of one. */
 
/*
 *  Data Structure Notes:
 *
 *  (1) Routines stored in sa_handler should take a single int as
 *      there argument although the POSIX standard does not require this.
 *  (2) The fields sa_handler and sa_sigaction may overlap, and a conforming
 *      application should not use both simultaneously.
 *
 *  NOTE:  In this implementation, macros are provided to access into the
 *         union.
 */
 
struct sigaction {
  int         sa_flags;       /* Special flags to affect behavior of signal */
  sigset_t    sa_mask;        /* Additional set of signals to be blocked */
                              /*   during execution of signal-catching */
                              /*   function. */
  union {
    void      (*_handler)();  /* SIG_DFL, SIG_IGN, or pointer to a function */
    void      (*_sigaction)( int, siginfo_t *, void * );
  } _signal_handlers;
};
 
#define sa_handler    _signal_handlers._handler
#define sa_sigaction  _signal_handlers._sigaction

#ifdef __cplusplus
}
#endif 

#endif
/* end of include file */
