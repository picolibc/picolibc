/* strsig.cc

   Copyright 2004 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <cygtls.h>

struct sigdesc
{
  int n;
  const char *name;
  const char *str;
};

#define __signals \
  _s(SIGHUP, "Hangup"),				/*  1 */ \
  _s(SIGINT, "Interrupt"),			/*  2 */ \
  _s(SIGQUIT, "Quit"),				/*  3 */ \
  _s(SIGILL, "Illegal instruction"),		/*  4 */ \
  _s(SIGTRAP, "Trace/breakpoint trap"),		/*  5 */ \
  _s(SIGABRT, "Aborted"),			/*  6 */ \
  _s(SIGEMT, "EMT instruction"),		/*  7 */ \
  _s(SIGFPE, "Floating point exception"),	/*  8 */ \
  _s(SIGKILL, "Killed"),			/*  9 */ \
  _s(SIGBUS, "Bus error"),			/* 10 */ \
  _s(SIGSEGV, "Segmentation fault"),		/* 11 */ \
  _s(SIGSYS, "Bad system call"),		/* 12 */ \
  _s(SIGPIPE, "Broken pipe"),			/* 13 */ \
  _s(SIGALRM, "Alarm clock"),			/* 14 */ \
  _s(SIGTERM, "Terminated"),			/* 15 */ \
  _s(SIGURG, "Urgent I/O condition"),		/* 16 */ \
  _s(SIGSTOP, "Stopped (signal)"),		/* 17 */ \
  _s(SIGTSTP, "Stopped"),			/* 18 */ \
  _s(SIGCONT, "Continued"),			/* 19 */ \
  _s2(SIGCHLD, "Child exited",			/* 20 */ \
      SIGCLD, "Child exited"),				 \
  _s(SIGTTIN, "Stopped (tty input)"),		/* 21 */ \
  _s(SIGTTOU, "Stopped (tty output)"),		/* 22 */ \
  _s2(SIGIO, "I/O possible",			/* 23 */ \
      SIGPOLL, "I/O possible"),				 \
  _s(SIGXCPU, "CPU time limit exceeded"),	/* 24 */ \
  _s(SIGXFSZ, "File size limit exceeded"),	/* 25 */ \
  _s(SIGVTALRM, "Virtual timer expired"),	/* 26 */ \
  _s(SIGPROF, "Profiling timer expired"),	/* 27 */ \
  _s(SIGWINCH, "Window changed"),		/* 28 */ \
  _s(SIGLOST, "Resource lost"),			/* 29 */ \
  _s(SIGUSR1, "User defined signal 1"),		/* 30 */ \
  _s(SIGUSR2, "User defined signal 2"),		/* 31 */ \
  _s2(SIGRTMIN, "Real-time signal 0",		/* 32 */ \
      SIGRTMAX, "Real-time signal 0")

#define _s(n, s) #n
#define _s2(n, s, n1, s1) #n
const char __declspec(dllexport) * sys_sigabbrev[] NO_COPY_INIT =
{
  NULL,
  __signals
};

#undef _s
#undef _s2
#define _s(n, s) {n, #n, s}
#define _s2(n, s, n1, s1) {n, #n, s}, {n, #n1, #s1}
static const sigdesc siglist[] =
{
  __signals,
  {0, NULL, NULL}
};

extern "C" const char *
strsignal (int signo)
{
  for (int i = 0; siglist[i].n; i++)
    if (siglist[i].n == signo)
      return siglist[i].str;
  __small_sprintf (_my_tls.locals.signamebuf, "Unknown signal %d", signo);
  return _my_tls.locals.signamebuf;
}

extern "C" int
strtosigno (const char *name)
{
  for (int i = 0; siglist[i].n; i++)
    if (strcmp (siglist[i].name, name) == 0)
      return siglist[i].n;
  return 0;
}
