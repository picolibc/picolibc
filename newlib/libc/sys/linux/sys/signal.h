/* libc/sys/linux/sys/signal.h - Signal handling */

/* Written 2000 by Werner Almesberger */


#ifndef _SYS_SIGNAL_H
#define _SYS_SIGNAL_H
#define _SIGNAL_H

#include <sys/types.h>
#include <bits/sigset.h>
#include <bits/signum.h>

/* we want RT signals so we must override the definition of sigset_t
   and NSIG */

#undef NSIG
#define NSIG _NSIG
#undef sigset_t
#define sigset_t __sigset_t

typedef void (*_sig_func_ptr) (int);
typedef _sig_func_ptr __sighandler_t;

#include <bits/siginfo.h>
#include <bits/sigaction.h>
#include <bits/sigstack.h>

/* --- include/signal.h thinks it knows better :-( --- */

#undef SIG_DFL
#undef SIG_IGN
#undef SIG_ERR

/* --- redundant stuff below --- */

#include <_ansi.h>

int 	kill (int, int);
int 	sigaction (int, const struct sigaction *, struct sigaction *);
int 	sigaddset (sigset_t *, const int);
int 	sigdelset (sigset_t *, const int);
int 	sigismember (const sigset_t *, int);
int 	sigfillset (sigset_t *);
int 	sigemptyset (sigset_t *);
int 	sigpending (sigset_t *);
int 	sigsuspend (const sigset_t *);
int 	sigpause (int);

#ifndef _POSIX_SOURCE
extern const char *const sys_siglist[];
#endif

#endif
