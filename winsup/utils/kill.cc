/* kill.cc

   Copyright 1996, 1997, 1998, 1999, 2000 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <windows.h>

static void usage (void);
static int __stdcall getsig (char *);
static void __stdcall forcekill (int, int);

int
main (int argc, char **argv)
{
  int sig = SIGTERM;
  int force = 0;
  int gotsig = 0;

  if (argc == 1)
    usage ();

  while (*++argv && **argv == '-')
    if (strcmp (*argv + 1, "f") == 0)
      force = 1;
    else if (gotsig)
      break;
    else if (strcmp(*argv + 1, "0") != 0)
      {
	sig = getsig (*argv + 1);
	gotsig = 1;
      }
    else
      {
	argv++;
	sig = 0;
	goto sig0;
      }

  if (sig <= 0 || sig > NSIG)
    {
      fprintf (stderr, "kill: unknown signal: %s\n", argv[-1]);
      exit (1);
    }

sig0:
  while (*argv != NULL)
    {
      char *p;
      int pid = strtol (*argv, &p, 10);
      if (*p != '\0')
	fprintf (stderr, "kill: illegal pid: %s\n", *argv);
      else
	{
#if 0
	  printf ("Sending %s(%d) signal to pid %d\n",
		  strsignal (sig), sig, pid);
#endif
	  if (kill (pid, sig))
	    {
	      if (errno == ESRCH && force && sig != 0)
		forcekill (pid, sig);
	      else
		{
		  char buf[1000];
		  sprintf (buf, "kill %d", pid);
		  perror (buf);
		}
	    }
	}
      argv++;
    }
  return 0;
}

static void
usage (void)
{
  fprintf (stderr, "Usage: kill [-sigN] pid1 [pid2 ...]\n");
  exit (1);
}

static int
getsig (char *in_sig)
{
  char *sig;
  char buf[80];

  if (strncmp (in_sig, "SIG", 3) == 0)
    sig = in_sig;
  else
    {
      sprintf (buf, "SIG%s", in_sig);
      sig = buf;
    }
  return (strtosigno (sig) ?: atoi (in_sig));
}

static void __stdcall
forcekill (int pid, int sig)
{
  HANDLE h = OpenProcess (PROCESS_TERMINATE, FALSE, (DWORD) pid);
  if (!h)
    return;
  TerminateProcess (h, sig << 8);
  CloseHandle (h);
}
