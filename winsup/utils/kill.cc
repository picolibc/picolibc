/* kill.cc

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.

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
#include <limits.h>

static const char version[] = "$Revision$";
static char *prog_name;

static struct option longopts[] =
{
  {"help", no_argument, NULL, 'h' },
  {"list", optional_argument, NULL, 'l'},
  {"force", no_argument, NULL, 'f'},
  {"signal", required_argument, NULL, 's'},
  {"version", no_argument, NULL, 'v'},
  {NULL, 0, NULL, 0}
};

static char opts[] = "hl::fs:v";

static void
usage (FILE *where = stderr)
{
  fprintf (where , ""
	"Usage: %s [-f] [-signal] [-s signal] pid1 [pid2 ...]\n"
	"       %s -l [signal]\n"
	"Send signals to processes\n"
	"\n"
	" -f, --force     force, using win32 interface if necessary\n"
	" -l, --list      print a list of signal names\n"
	" -s, --signal    send signal (use %s --list for a list)\n"
	" -h, --help      output usage information and exit\n"
	" -v, --version   output version information and exit\n"
	"", prog_name, prog_name, prog_name);
  exit (where == stderr ? 1 : 0);
}

static void
print_version ()
{
  const char *v = strchr (version, ':');
  int len;
  if (!v)
    {
      v = "?";
      len = 1;
    }
  else
    {
      v += 2;
      len = strchr (v, ' ') - v;
    }
  printf ("\
%s (cygwin) %.*s\n\
Process Signaller\n\
Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.\n\
Compiled on %s\n\
", prog_name, len, v, __DATE__);
}

static const char *
strsigno (int signo)
{
  if (signo >= 0 && signo < NSIG)
    return sys_sigabbrev[signo];
  static char buf[sizeof ("Unknown signal") + 32];
  sprintf (buf, "Unknown signal %d", signo);
  return buf;
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
      sprintf (buf, "SIG%-.20s", in_sig);
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
      fprintf (stderr, "%s: unknown signal: %s\n", prog_name, sigstr);
      usage ();
      exit (1);
    }
}

static void
listsig (const char *in_sig)
{
  int sig;
  if (!in_sig)
    for (sig = 1; sig < NSIG - 1; sig++)
      printf ("%s%c", strsigno (sig) + 3, (sig < NSIG - 1) ? ' ' : '\n');
  else
    {
      sig = getsig (in_sig);
      test_for_unknown_sig (sig, in_sig);
      if (atoi (in_sig) == sig)
	puts (strsigno (sig) + 3);
      else
	printf ("%d\n", sig);
    }
}

static void
get_debug_priv (void)
{
  HANDLE tok;
  LUID luid;
  TOKEN_PRIVILEGES tkp;

  if (!OpenProcessToken (GetCurrentProcess (),
  			 TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &tok))
    return;

  if (!LookupPrivilegeValue (NULL, SE_DEBUG_NAME, &luid))
    {
      CloseHandle (tok);
      return;
    }

  tkp.PrivilegeCount = 1;
  tkp.Privileges[0].Luid = luid;
  tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

  AdjustTokenPrivileges (tok, FALSE, &tkp, sizeof tkp, NULL, NULL);

  CloseHandle (tok);
}

static void __stdcall
forcekill (int pid, int sig, int wait)
{
  // try to acquire SeDebugPrivilege
  get_debug_priv();

  external_pinfo *p = NULL;
  /* cygwin_internal misinterprets negative pids (Win9x pids) */
  if (pid > 0)
    p = (external_pinfo *) cygwin_internal (CW_GETPINFO_FULL, pid);
  DWORD dwpid = p ? p->dwProcessId : (DWORD) pid;
  HANDLE h = OpenProcess (PROCESS_TERMINATE, FALSE, (DWORD) dwpid);
  if (!h)
    {
      if (!wait || GetLastError () != ERROR_INVALID_PARAMETER) 
        fprintf (stderr, "%s: couldn't open pid %u\n", 
	         prog_name, (unsigned) dwpid);
      return;
    }
  if (!wait || WaitForSingleObject (h, 200) != WAIT_OBJECT_0)
    if (sig && !TerminateProcess (h, sig << 8)
	&& WaitForSingleObject (h, 200) != WAIT_OBJECT_0)
      fprintf (stderr, "%s: couldn't kill pid %u, %lu\n", 
	       prog_name, (unsigned) dwpid, GetLastError ());
  CloseHandle (h);
}

int
main (int argc, char **argv)
{
  int sig = SIGTERM;
  int force = 0;
  int ret = 0;
  char *gotasig = NULL;

  prog_name = strrchr (argv[0], '/');
  if (prog_name == NULL)
    prog_name = strrchr (argv[0], '\\');
  if (prog_name == NULL)
    prog_name = argv[0];
  else
    prog_name++;

  if (argc == 1)
    usage ();

  opterr = 0;

  char *p;
  long long int pid = 0;

  for (;;)
    {
      int ch;
      char **av = argv + optind;
      if ((ch = getopt_long (argc, argv, opts, longopts, NULL)) == EOF)
	break;
      switch (ch)
	{
	case 's':
	  gotasig = optarg;
	  sig = getsig (gotasig);
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
	case 'v':
	  print_version ();
	  break;
	case '?':
	  if (gotasig)
	    {
	      --optind;
	      goto out;
	    }
	  optreset = 1;
	  optind = 1 + av - argv;
	  gotasig = *av + 1;
	  sig = getsig (gotasig);
	  break;
	default:
	  usage ();
	  break;
	}
    }

out:
  test_for_unknown_sig (sig, gotasig);

  argv += optind;
  while (*argv != NULL)
    {
      if (!pid)
	pid = strtoll (*argv, &p, 10);
      if (*p != '\0'
          || (!force && (pid < LONG_MIN || pid > LONG_MAX))
          || (force && (pid <= 0 || pid > ULONG_MAX))) 
	{
	  fprintf (stderr, "%s: illegal pid: %s\n", prog_name, *argv);
	  ret = 1;
	}
      else if (pid <= LONG_MAX && kill ((pid_t) pid, sig) == 0)
	{
	  if (force)
	    forcekill ((pid_t) pid, sig, 1);
	}
      else if (force)
	forcekill ((pid_t) pid, sig, 0);
      else
	{
	  char buf[1000];
	  sprintf (buf, "%s: %lld", prog_name, pid);
	  perror (buf);
	  ret = 1;
	}
      argv++;
      pid = 0;
    }
  return ret;
}

