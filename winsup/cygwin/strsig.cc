/* strsig.cc

   Copyright 2004 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "thread.h"
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <cygtls.h>

struct sigdesc
{
  int n;
  const char *name;
  const char *str;
};

#define _s(n, s) {n, #n, s}
static const sigdesc siglist[] =
{
  _s(SIGHUP, "Hangup"),
  _s(SIGINT, "Interrupt"),
  _s(SIGQUIT, "Quit"),
  _s(SIGILL, "Illegal instruction"),
  _s(SIGTRAP, "Trace/breakpoint trap"),
  _s(SIGABRT, "Aborted"),
  _s(SIGEMT, "EMT instruction"),
  _s(SIGFPE, "Floating point exception"),
  _s(SIGKILL, "Killed"),
  _s(SIGBUS, "Bus error"),
  _s(SIGSEGV, "Segmentation fault"),
  _s(SIGSYS, "Bad system call"),
  _s(SIGPIPE, "Broken pipe"),
  _s(SIGALRM, "Alarm clock"),
  _s(SIGTERM, "Terminated"),
  _s(SIGURG, "Urgent I/O condition"),
  _s(SIGSTOP, "Stopped (signal)"),
  _s(SIGTSTP, "Stopped"),
  _s(SIGCONT, "Continued"),
  _s(SIGCHLD, "Child exited"),
  _s(SIGCLD, "Child exited"),
  _s(SIGTTIN, "Stopped (tty input)"),
  _s(SIGTTOU, "Stopped (tty output)"),
  _s(SIGIO, "I/O possible"),
  _s(SIGPOLL, "I/O possible"),
  _s(SIGXCPU, "CPU time limit exceeded"),
  _s(SIGXFSZ, "File size limit exceeded"),
  _s(SIGVTALRM, "Virtual timer expired"),
  _s(SIGPROF, "Profiling timer expired"),
  _s(SIGWINCH, "Window changed"),
  _s(SIGLOST, "Resource lost"),
  _s(SIGUSR1, "User defined signal 1"),
  _s(SIGUSR2, "User defined signal 2"),
  _s(SIGRTMIN, "Real-time signal 0"),
  _s(SIGRTMAX, "Real-time signal 0"),
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
