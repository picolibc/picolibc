/* kill.cc

   Copyright 1996, 1997, 1998 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <time.h>

static void usage (void);
static int getsig (char *);
int a = _timezone;

int
main (int ac, char **av)
{
  int sig = SIGTERM;

  if (ac == 1)
    usage ();

  if (*(++av)[0] == '-')
    if (strcmp(*av + 1, "0") != 0)
      sig = getsig (*av++ + 1);
    else
      {
	av++;
	sig = 0;
	goto sig0;
      }

  if (sig <= 0 || sig > NSIG)
    {
      fprintf (stderr, "kill: unknown signal: %s\n", av[-1]);
      exit (1);
    }

sig0:
  while (*av != NULL)
    {
      char *p;
      int pid = strtol (*av, &p, 10);
      if (*p != '\0')
	fprintf (stderr, "kill: illegal pid: %s\n", *av);
      else
	{
	  printf ("Sending %s(%d) signal to pid %d\n",
		  strsignal (sig), sig, pid);
	  if (kill (pid, sig))
	    perror ("kill");
	}
      av++;
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
