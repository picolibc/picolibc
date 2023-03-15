/* kill.cc

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
#include <cygwin/version.h>
#include <getopt.h>
#include <limits.h>

static char *prog_name;

static struct option longopts[] =
{
  {"help", no_argument, NULL, 'h' },
  {"list", optional_argument, NULL, 'l'},
  {"force", no_argument, NULL, 'f'},
  {"signal", required_argument, NULL, 's'},
  {"table", no_argument, NULL, 'L'},
  {"winpid", no_argument, NULL, 'W'},
  {"version", no_argument, NULL, 'V'},
  {NULL, 0, NULL, 0}
};

static char opts[] = "hl::fs:LWV";

static void __attribute__ ((__noreturn__))
usage (FILE *where = stderr)
{
  fprintf (where , ""
	"Usage: %1$s [-fW] [-signal] [-s signal] pid1 [pid2 ...]\n"
	"       %1$s -l [signal] | -L\n"
	"\n"
	"Send signals to processes\n"
	"\n"
	" -f, --force     force, using win32 interface if necessary\n"
	" -l, --list      print a list of signal names\n"
	" -L, --table     print a formatted table of signal names\n"
	" -s, --signal    send signal (use %1$s --list for a list)\n"
	" -W, --winpid    specified pids are windows PIDs, not Cygwin PIDs\n"
	"                 (use with extreme caution!)\n"
	" -h, --help      output usage information and exit\n"
	" -V, --version   output version information and exit\n"
	"\n", prog_name);
  exit (where == stderr ? 1 : 0);
}

static void
print_version ()
{
  printf ("kill (cygwin) %d.%d.%d\n"
	  "Process Signaller\n"
	  "Copyright (C) 1996 - %s Cygwin Authors\n"
	  "This is free software; see the source for copying conditions.  There is NO\n"
	  "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n",
	  CYGWIN_VERSION_DLL_MAJOR / 1000,
	  CYGWIN_VERSION_DLL_MAJOR % 1000,
	  CYGWIN_VERSION_DLL_MINOR,
	  strrchr (__DATE__, ' ') + 1);
  exit (0);
}

static const char *
strsigno (int signo)
{
  static char sigbuf[8];

  if (signo > 0 && signo < SIGRTMIN)
    return sys_sigabbrev[signo];
  if (signo <= SIGRTMAX)
    {
      snprintf (sigbuf, sizeof sigbuf, "SIGRT%d", signo - SIGRTMIN);
      return sigbuf;
    }
  static char buf[sizeof ("Unknown signal") + 32];
  sprintf (buf, "Unknown signal %d", signo);
  return buf;
}

static int
strtortsig (const char *sig)
{
  bool neg = false;
  char *endp = NULL;
  int signo;

  sig += 5;
  if (!strcmp (sig, "MIN"))
    return SIGRTMIN;
  if (!strcmp (sig, "MAX"))
    return SIGRTMAX;
  if (!strncmp (sig, "MIN+", 4))
    sig += 4;
  else if (!strncmp (sig, "MAX-", 4))
    {
      sig += 4;
      neg = true;
    }
  signo = strtoul (sig, &endp, 10);
  if (!endp || *endp)
    return 0;
  if (neg)
    signo = SIGRTMAX - signo;
  else
    signo = SIGRTMIN + signo;
  if (signo < SIGRTMIN || signo > SIGRTMAX)
    return 0;
  return signo;
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
  if (!strncmp (sig, "SIGRT", 5))
    intsig = strtortsig (sig);
  else
    intsig = strtosigno (sig) ?: atoi (in_sig);
  char *p;
  if (!intsig && (strcmp (sig, "SIG0") != 0 && (strtol (in_sig, &p, 10) != 0 || *p)))
    intsig = -1;
  return intsig;
}

static void
test_for_unknown_sig (int sig, const char *sigstr)
{
  if (sig < 0 || sig > NSIG)
    {
      fprintf (stderr, "%1$s: unknown signal: %2$s\n"
		       "Try `%1$s --help' for more information.\n",
	       prog_name, sigstr);
      exit (1);
    }
}

static void
checksig (const char *in_sig)
{
  int sig = getsig (in_sig);
  test_for_unknown_sig (sig, in_sig);
  if (sig && atoi (in_sig) == sig)
    puts (strsigno (sig) + 3);
  else
    printf ("%d\n", sig);
  exit (0);
}

static void
listsig ()
{
  int chars = 0;

  for (int sig = 1; sig < SIGRTMIN; sig++)
    {
      chars += printf ("%s ", strsigno (sig) + 3);
      if (chars > 72)
	{
	  puts ("");
	  chars = 0;
	}
      switch (sig)
	{
	case SIGABRT:
	  chars += printf ("%s ", "IOT");
	  break;
	case SIGCHLD:
	  chars += printf ("%s ", "CLD");
	  break;
	case SIGIO:
	  chars += printf ("%s ", "POLL");
	  break;
	case SIGPWR:
	  chars += printf ("%s ", "LOST");
	  break;
	}
      if (chars > 70)
	{
	  puts ("");
	  chars = 0;
	}
    }
  fputs ("RT<N> RTMIN+<N> RTMAX-<N>\n", stdout);
  exit (0);
}

static void
tablesig ()
{
  int chars = 0;

  for (int sig = 1; sig < SIGRTMIN; sig++)
    {
      chars += printf ("%2d %-7s ", sig, strsigno (sig) + 3);
      if (chars > 70)
	{
	  puts ("");
	  chars = 0;
	}
      switch (sig)
	{
	case SIGABRT:
	  chars += printf ("%2d %-7s ", sig, "IOT");
	  break;
	case SIGCHLD:
	  chars += printf ("%2d %-7s ", sig, "CLD");
	  break;
	case SIGIO:
	  chars += printf ("%2d %-7s ", sig, "POLL");
	  break;
	case SIGPWR:
	  chars += printf ("%2d %-7s ", sig, "LOST");
	  break;
	}
      if (chars > 70)
	{
	  puts ("");
	  chars = 0;
	}
    }
  fputs ("32 RTMIN   64 RTMAX\n", stdout);
  exit (0);
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

static void
forcekill (pid_t pid, DWORD winpid, int sig, int wait)
{
  DWORD dwpid;

  /* try to acquire SeDebugPrivilege */
  get_debug_priv();

  if (!winpid)
    {
      external_pinfo *p = (external_pinfo *)
			  cygwin_internal (CW_GETPINFO_FULL, pid);
      if (!p)
	{
	  fprintf (stderr, "%s: %d: No such process\n", prog_name, pid);
	  return;
	}
      dwpid = p->dwProcessId;
    }
  else
    /* pid is used for printing only after this point */
    pid = dwpid = winpid;

  HANDLE h = OpenProcess (PROCESS_TERMINATE, FALSE, dwpid);
  if (!h)
    {
      if (!wait || GetLastError () != ERROR_INVALID_PARAMETER)
	fprintf (stderr, "%s: couldn't open pid %u\n",
		 prog_name, (unsigned int) dwpid);
      return;
    }
  if (!wait || WaitForSingleObject (h, 200) != WAIT_OBJECT_0)
    if (sig && !TerminateProcess (h, sig << 8)
	&& WaitForSingleObject (h, 200) != WAIT_OBJECT_0)
      fprintf (stderr, "%s: couldn't kill pid %u, %u\n",
	       prog_name, (unsigned int) dwpid, (unsigned int) GetLastError ());
  CloseHandle (h);
}

int
main (int argc, char **argv)
{
  int sig = SIGTERM;
  int force = 0;
  int winpids = 0;
  int ret = 0;
  char *gotasig = NULL;

  prog_name = program_invocation_short_name;

  if (argc == 1)
    usage ();

  opterr = 0;

  char *p;

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
	  if (optarg)
	    checksig (optarg);
	  else
	    listsig ();
	  break;
	case 'L':
	  tablesig ();
	  break;
	case 'f':
	  force = 1;
	  break;
	case 'W':
	  winpids = 1;
	  break;
	case 'h':
	  usage (stdout);
	  break;
	case 'V':
	  print_version ();
	  break;
	case '?':
	  if (gotasig) /* this is a negative pid, go ahead */
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
  if (*argv == 0)
    {
      fprintf (stderr, "%s: not enough arguments\n", prog_name);
      return 1;
    }
  for (long long int pid = 0; *argv != NULL; argv++)
    {
      DWORD dwpid = 0;

      pid = strtoll (*argv, &p, 10);
      /* INT_MIN <= pid <= INT_MAX.  -f only takes positive pids. */
      if (*p != '\0' || pid < (force ? 1 : INT_MIN) || pid > INT_MAX)
	{
	  fprintf (stderr, "%s: illegal pid: %s\n", prog_name, *argv);
	  ret = 1;
	  continue;
	}
      if (winpids)
	{
	  dwpid = pid;
	  pid = (pid_t) cygwin_internal (CW_WINPID_TO_CYGWIN_PID, dwpid);
	}
      if (kill ((pid_t) pid, sig) == 0)
	{
	  if (force)
	    forcekill ((pid_t) pid, dwpid, sig, 1);
	}
      else if (force)
	forcekill ((pid_t) pid, dwpid, sig, 0);
      else
	{
	  char buf[1000];
	  sprintf (buf, "%s: %lld", prog_name, dwpid ?: pid);
	  perror (buf);
	  ret = 1;
	}
    }
  return ret;
}

