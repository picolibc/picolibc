/* kill.cc

   Copyright 1996, 1997, 1998, 1999, 2000, 2001 Red Hat, Inc.

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
#include <sys/cygwin.h>
#include <getopt.h>

static struct option longopts[] =
{
  {"help", no_argument, NULL, 'h' },
  {"list", optional_argument, NULL, 'l'},
  {"force", no_argument, NULL, 'f'},
  {"signal", required_argument, NULL, 's'},
  {NULL, 0, NULL, 0}
};

static char opts[] = "hl::fs:";

extern "C" const char *strsigno (int);

static void
usage (FILE *where = stderr)
{
  fputs ("usage: kill [-signal] [-s signal] pid1 [pid2 ...]\n"
	 "       kill -l [signal]\n", where);
  exit (where == stderr ? 1 : 0);
}

static int
getsig (const char *in_sig)
{
  const char *sig;
  char buf[80];
  int intsig;

  if (strncmp (in_sig, "SIG", 3) == 0)
    sig = in_sig;
  else
    {
      sprintf (buf, "SIG%s", in_sig);
      sig = buf;
    }
  intsig = strtosigno (sig) ?: atoi (in_sig);
  char *p;
  if (!intsig && (strcmp (buf, "SIG0") != 0 && (strtol (in_sig, &p, 10) != 0 || *p)))
    intsig = -1;
  return intsig;
}

static void
test_for_unknown_sig (int sig, const char *sigstr)
{
  if (sig < 0 || sig > NSIG)
    {
      fprintf (stderr, "kill: unknown signal: %s\n", sigstr);
      usage ();
      exit (1);
    }
}

static void
listsig (const char *in_sig)
{
  int sig;
  if (!in_sig)
    for (sig = 1; sig < NSIG; sig++)
      printf ("%s%c", strsigno (sig) + 3, (sig < NSIG - 1) ? ' ' : '\n');
  else
    {
      sig = getsig (in_sig);
      test_for_unknown_sig (sig, in_sig);
      puts (strsigno (sig) + 3);
    }
}

static void __stdcall
forcekill (int pid, int sig, int wait)
{
  external_pinfo *p = (external_pinfo *) cygwin_internal (CW_GETPINFO_FULL, pid);
  if (!p)
    return;
  HANDLE h = OpenProcess (PROCESS_TERMINATE, FALSE, (DWORD) p->dwProcessId);
  if (!h)
    return;
  if (!wait || WaitForSingleObject (h, 200) != WAIT_OBJECT_0)
    TerminateProcess (h, sig << 8);
  CloseHandle (h);
}

int
main (int argc, char **argv)
{
  int sig = SIGTERM;
  int force = 0;
  char *gotsig = NULL;
  int ret = 0;

  if (argc == 1)
    usage ();

  opterr = 0;
  for (;;)
    {
      int ch;
      char **av = argv + optind;
      if ((ch = getopt_long (argc, argv, opts, longopts, NULL)) == EOF)
	break;
      switch (ch)
	{
	case 's':
	  gotsig = optarg;
	  sig = getsig (gotsig);
	  break;
	case 'l':
	  if (!optarg)
	    {
	      optarg = argv[optind];
	      if (optarg)
		{
		  optind++;
		  optreset = 1;
		}
	    }
	  if (argv[optind])
	    usage ();
	  listsig (optarg);
	  break;
	case 'f':
	  force = 1;
	  break;
	case 'h':
	  usage (stdout);
	  break;
	case '?':
	  if (gotsig)
	    usage ();
	  optreset = 1;
	  optind = 1 + av - argv;
	  gotsig = *av + 1;
	  sig = getsig (gotsig);
	  break;
	default:
	  usage ();
	  break;
	}
    }

  test_for_unknown_sig (sig, gotsig);

  argv += optind;
  while (*argv != NULL)
    {
      char *p;
      int pid = strtol (*argv, &p, 10);
      if (*p != '\0')
	{
	  fprintf (stderr, "kill: illegal pid: %s\n", *argv);
	  ret = 1;
	}
      else if (kill (pid, sig) == 0)
	{
	  if (force)
	    forcekill (pid, sig, 1);
	}
      else if (force && sig != 0)
	forcekill (pid, sig, 0);
      else
	{
	  char buf[1000];
	  sprintf (buf, "kill %d", pid);
	  perror (buf);
	  ret = 1;
	}
      argv++;
    }
  return ret;
}
