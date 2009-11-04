/* strace.cc

   Copyright 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008,
   2009 Red Hat Inc.

   Written by Chris Faylor <cgf@redhat.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#define cygwin_internal cygwin_internal_dontuse
#include <stdio.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <signal.h>
#include <errno.h>
#include "cygwin/include/sys/strace.h"
#include "cygwin/include/sys/cygwin.h"
#include "path.h"
#undef cygwin_internal

/* we *know* we're being built with GCC */
#define alloca __builtin_alloca

// Version string.
static const char version[] = "$Revision$";

static const char *pgm;
static int forkdebug = 1;
static int numerror = 1;
static int show_usecs = 1;
static int delta = 1;
static int hhmmss;
static int bufsize;
static int new_window;
static long flush_period;
static int include_hex;
static int quiet = -1;

static unsigned char strace_active = 1;
static int processes;

static BOOL close_handle (HANDLE h, DWORD ok);

#define CloseHandle(h) close_handle(h, 0)

struct child_list
{
  DWORD id;
  HANDLE hproc;
  int saw_stars;
  char nfields;
  long long start_time;
  DWORD last_usecs;
  struct child_list *next;
    child_list ():id (0), hproc (NULL), saw_stars (0), nfields (0),
    start_time (0), last_usecs (0), next (NULL)
  {
  }
};

child_list children;

static void
warn (int geterrno, const char *fmt, ...)
{
  va_list args;
  char buf[4096];

  va_start (args, fmt);
  sprintf (buf, "%s: ", pgm);
  vsprintf (strchr (buf, '\0'), fmt, args);
  if (geterrno)
    perror (buf);
  else
    {
      fputs (buf, stderr);
      fputs ("\n", stderr);
    }
}

static void __attribute__ ((noreturn))
error (int geterrno, const char *fmt, ...)
{
  va_list args;
  char buf[4096];

  va_start (args, fmt);
  sprintf (buf, "%s: ", pgm);
  vsprintf (strchr (buf, '\0'), fmt, args);
  if (geterrno)
    perror (buf);
  else
    {
      fputs (buf, stderr);
      fputs ("\n", stderr);
    }
  exit (1);
}

DWORD lastid = 0;
HANDLE lasth;

static child_list *
get_child (DWORD id)
{
  child_list *c;
  for (c = &children; (c = c->next) != NULL;)
    if (c->id == id)
      return c;

  return NULL;
}

static void
add_child (DWORD id, HANDLE hproc)
{
  if (!get_child (id))
    {
      child_list *c = children.next;
      children.next = (child_list *) calloc (1, sizeof (child_list));
      children.next->next = c;
      lastid = children.next->id = id;
      lasth = children.next->hproc = hproc;
      processes++;
      if (!quiet)
	fprintf (stderr, "Windows process %lu attached\n", id);
    }
}

static void
remove_child (DWORD id)
{
  child_list *c;
  if (id == lastid)
    lastid = 0;
  for (c = &children; c->next != NULL; c = c->next)
    if (c->next->id == id)
      {
	child_list *c1 = c->next;
	c->next = c1->next;
	free (c1);
	if (!quiet)
	  fprintf (stderr, "Windows process %lu detached\n", id);
	processes--;
	return;
      }

  error (0, "no process id %d found", id);
}

#define LINE_BUF_CHUNK 128

class linebuf
{
  size_t alloc;
public:
    size_t ix;
  char *buf;
  linebuf ()
  {
    ix = 0;
    alloc = 0;
    buf = NULL;
  }
 ~linebuf ()
  {
    if (buf)
      free (buf);
  }
  void add (const char *what, int len);
  void add (const char *what)
  {
    add (what, strlen (what));
  }
  void prepend (const char *what, int len);
};

void
linebuf::add (const char *what, int len)
{
  size_t newix;
  if ((newix = ix + len) >= alloc)
    {
      alloc += LINE_BUF_CHUNK + len;
      buf = (char *) realloc (buf, alloc + 1);
    }
  memcpy (buf + ix, what, len);
  ix = newix;
  buf[ix] = '\0';
}

void
linebuf::prepend (const char *what, int len)
{
  int buflen;
  size_t newix;
  if ((newix = ix + len) >= alloc)
    {
      alloc += LINE_BUF_CHUNK + len;
      buf = (char *) realloc (buf, alloc + 1);
      buf[ix] = '\0';
    }
  if ((buflen = strlen (buf)))
    memmove (buf + len, buf, buflen + 1);
  else
    buf[newix] = '\0';
  memcpy (buf, what, len);
  ix = newix;
}

static void
make_command_line (linebuf & one_line, char **argv)
{
  for (; *argv; argv++)
    {
      char *p = NULL;
      const char *a = *argv;

      int len = strlen (a);
      if (len != 0 && !(p = strpbrk (a, " \t\n\r\"")))
	one_line.add (a, len);
      else
	{
	  one_line.add ("\"", 1);
	  for (; p; a = p, p = strchr (p, '"'))
	    {
	      one_line.add (a, ++p - a);
	      if (p[-1] == '"')
		one_line.add ("\"", 1);
	    }
	  if (*a)
	    one_line.add (a);
	  one_line.add ("\"", 1);
	}
      one_line.add (" ", 1);
    }

  if (one_line.ix)
    one_line.buf[one_line.ix - 1] = '\0';
  else
    one_line.add ("", 1);
}

static DWORD child_pid;

static BOOL WINAPI
ctrl_c (DWORD)
{
  static int tic = 1;
  if ((tic ^= 1) && !GenerateConsoleCtrlEvent (CTRL_C_EVENT, 0))
    error (0, "couldn't send CTRL-C to child, win32 error %d\n",
	   GetLastError ());
  return TRUE;
}

extern "C" {
unsigned long (*cygwin_internal) (int, ...);
WCHAR cygwin_dll_path[32768];
};

static int
load_cygwin ()
{
  static HMODULE h;

  if (cygwin_internal)
    return 1;

  if (h)
    return 0;

  if (!(h = LoadLibrary ("cygwin1.dll")))
    {
      errno = ENOENT;
      return 0;
    }
  GetModuleFileNameW (h, cygwin_dll_path, 32768);
  if (!(cygwin_internal = (DWORD (*) (int, ...)) GetProcAddress (h, "cygwin_internal")))
    {
      errno = ENOSYS;
      return 0;
    }
  return 1;
}

static void
attach_process (pid_t pid)
{
  child_pid = (DWORD) cygwin_internal (CW_CYGWIN_PID_TO_WINPID, pid);
  if (!child_pid)
    child_pid = pid;

  if (!DebugActiveProcess (child_pid))
    error (0, "couldn't attach to pid %d for debugging", child_pid);

  return;
}


static void
create_child (char **argv)
{
  linebuf one_line;

  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  BOOL ret;
  DWORD flags;

  if (strchr (*argv, '/'))
      *argv = cygpath (*argv, NULL);
  memset (&si, 0, sizeof (si));
  si.cb = sizeof (si);

  flags = CREATE_DEFAULT_ERROR_MODE
	  | (forkdebug ? DEBUG_PROCESS : DEBUG_ONLY_THIS_PROCESS);
  if (new_window)
    flags |= CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP;

  make_command_line (one_line, argv);

  SetConsoleCtrlHandler (NULL, 0);
  const char *cygwin_env = getenv ("CYGWIN");
  const char *space;
  if (cygwin_env)
    space = " ";
  else
    space = cygwin_env = "";
  char *newenv = (char *) malloc (sizeof ("CYGWIN=noglob") + strlen (space) + strlen (cygwin_env));
  sprintf (newenv, "CYGWIN=noglob%s%s", space, cygwin_env);
  _putenv (newenv);
  ret = CreateProcess (0, one_line.buf,	/* command line */
		       NULL,	/* Security */
		       NULL,	/* thread */
		       TRUE,	/* inherit handles */
		       flags,	/* start flags */
		       NULL,	/* default environment */
		       NULL,	/* current directory */
		       &si, &pi);
  if (!ret)
    error (0, "error creating process %s, (error %d)", *argv,
	   GetLastError ());

  CloseHandle (pi.hThread);
  CloseHandle (pi.hProcess);
  child_pid = pi.dwProcessId;
  SetConsoleCtrlHandler (ctrl_c, 1);
}

static int
output_winerror (FILE *ofile, char *s)
{
  char *winerr = strstr (s, "Win32 error ");
  if (!winerr)
    return 0;

  DWORD errnum = atoi (winerr + sizeof ("Win32 error ") - 1);
  if (!errnum)
    return 0;

  /*
   * NOTE: Currently there is no policy for how long the
   * the buffers are, and looks like 256 is a smallest one
   * (dlfcn.cc). Other than error 1395 (length 213) and
   * error 1015 (length 249), the rest are all under 188
   * characters, and so I'll use 189 as the buffer length.
   * For those longer error messages, FormatMessage will
   * return FALSE, and we'll get the old behaviour such as
   * ``Win32 error 1395'' etc.
   */
  char buf[4096];
  if (!FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM
		      | FORMAT_MESSAGE_IGNORE_INSERTS,
		      NULL,
		      errnum,
		      MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
		      (LPTSTR) buf, sizeof (buf), NULL))
    return 0;

  /* Get rid the trailing CR/NL pair. */
  char *p = strchr (buf, '\0');
  p[-2] = '\n';
  p[-1] = '\0';

  *winerr = '\0';
  fputs (s, ofile);
  fputs (buf, ofile);
  return 1;
}

static SYSTEMTIME *
syst (long long t)
{
  FILETIME n;
  static SYSTEMTIME st;
  long long now = t /*+ ((long long) usecs * 10)*/;
  n.dwHighDateTime = now >> 32;
  n.dwLowDateTime = now & 0xffffffff;
  FileTimeToSystemTime (&n, &st);
  return &st;
}

static void __stdcall
handle_output_debug_string (DWORD id, LPVOID p, unsigned mask, FILE *ofile)
{
  int len;
  int special;
  char alen[3 + 8 + 1];
  DWORD nbytes;
  child_list *child = get_child (id);
  if (!child)
    error (0, "no process id %d found", id);
  HANDLE hchild = child->hproc;
#define INTROLEN (sizeof (alen) - 1)

  if (id == lastid && hchild != lasth)
    warn (0, "%p != %p", hchild, lasth);

  alen[INTROLEN] = '\0';
  if (!ReadProcessMemory (hchild, p, alen, INTROLEN, &nbytes))
#ifndef DEBUGGING
    return;
#else
    error (0,
	   "couldn't get message length from subprocess %d<%p>, windows error %d",
	   id, hchild, GetLastError ());
#endif

  if (strncmp (alen, "cYg", 3))
    return;
  len = (int) strtoul (alen + 3, NULL, 16);
  if (!len)
    return;

  if (len > 0)
    special = 0;
  else
    {
      special = len;
      if (special == _STRACE_INTERFACE_ACTIVATE_ADDR || special == _STRACE_CHILD_PID)
	len = 17;
    }

  char *buf;
  buf = (char *) alloca (len + 85) + 20;

  if (!ReadProcessMemory (hchild, ((char *) p) + INTROLEN, buf, len, &nbytes))
    error (0, "couldn't get message from subprocess, windows error %d",
	   GetLastError ());

  buf[len] = '\0';
  char *s = strtok (buf, " ");

  unsigned long n = strtoul (s, NULL, 16);

  s = strchr (s, '\0') + 1;

  if (special == _STRACE_CHILD_PID)
    {
      if (!DebugActiveProcess (n))
	error (0, "couldn't attach to subprocess %d for debugging, "
	       "windows error %d", n, GetLastError ());
      return;
    }

  if (special == _STRACE_INTERFACE_ACTIVATE_ADDR)
    {
      if (!WriteProcessMemory (hchild, (LPVOID) n, &strace_active,
			       sizeof (strace_active), &nbytes))
	error (0, "couldn't write strace flag to subprocess at %p, "
	       "windows error %d", n, GetLastError ());
      return;
    }

  char *origs = s;

  if (mask & n)
    /* got it */ ;
  else if (!(mask & _STRACE_ALL) || (n & _STRACE_NOTALL))
    return;			/* This should not be included in "all" output */

  DWORD dusecs, usecs;
  char *ptusec, *ptrest;

  dusecs = strtoul (s, &ptusec, 10);
  char *q = ptusec;
  while (*q == ' ')
    q++;
  if (*q != '[')
    {
      usecs = strtoul (q, &ptrest, 10);
      while (*ptrest == ' ')
	ptrest++;
    }
  else
    {
      ptrest = q;
      ptusec = show_usecs ? s : ptrest;
      usecs = dusecs;
    }

  if (child->saw_stars == 0)
    {
      FILETIME st;
      char *news;

      GetSystemTimeAsFileTime (&st);
      FileTimeToLocalFileTime (&st, &st);
      child->start_time = st.dwHighDateTime;
      child->start_time <<= 32;
      child->start_time |= st.dwLowDateTime;
      if (*(news = ptrest) != '[')
	child->saw_stars = 2;
      else
	{
	  child->saw_stars++;
	  while ((news = strchr (news, ' ')) != NULL && *++news != '*')
	    child->nfields++;
	  if (news == NULL)
	    child->saw_stars++;
	  else
	    {
	      s = news;
	      child->nfields++;
	    }
	}
    }
  else if (child->saw_stars < 2)
    {
      int i;
      char *news;
      if (*(news = ptrest) != '[')
	child->saw_stars = 2;
      else
	{
	  for (i = 0; i < child->nfields; i++)
	    if ((news = strchr (news, ' ')) == NULL)
	      break;		// Should never happen
	    else
	      news++;

	  if (news == NULL)
	    child->saw_stars = 2;
	  else
	    {
	      s = news;
	      if (*s == '*')
		{
		  SYSTEMTIME *st = syst (child->start_time);
		  fprintf (ofile,
			   "Date/Time:    %d-%02d-%02d %02d:%02d:%02d\n",
			   st->wYear, st->wMonth, st->wDay, st->wHour,
			   st->wMinute, st->wSecond);
		  child->saw_stars++;
		}
	    }
	}
    }

  long long d = usecs - child->last_usecs;
  char intbuf[40];

  if (child->saw_stars < 2 || s != origs)
    /* Nothing */ ;
  else if (hhmmss)
    {
      s = ptrest - 9;
      SYSTEMTIME *st = syst (child->start_time + (long long) usecs * 10);
      sprintf (s, "%02d:%02d:%02d", st->wHour, st->wMinute, st->wSecond);
      *strchr (s, '\0') = ' ';
    }
  else if (!delta)
    s = ptusec;
  else
    {
      s = ptusec;
      sprintf (intbuf, "%5d ", (int) d);
      int len = strlen (intbuf);

      memcpy ((s -= len), intbuf, len);
    }

  if (include_hex)
    {
      s -= 8;
      sprintf (s, "%p", (void *) n);
      strchr (s, '\0')[0] = ' ';
    }
  child->last_usecs = usecs;
  if (numerror || !output_winerror (ofile, s))
    fputs (s, ofile);
  if (!bufsize)
    fflush (ofile);
}

static DWORD
proc_child (unsigned mask, FILE *ofile, pid_t pid)
{
  DWORD res = 0;
  DEBUG_EVENT ev;
  time_t cur_time, last_time;

  SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_HIGHEST);
  last_time = time (NULL);
  while (1)
    {
      BOOL debug_event = WaitForDebugEvent (&ev, 1000);
      DWORD status = DBG_CONTINUE;

      if (bufsize && flush_period > 0 &&
	  (cur_time = time (NULL)) >= last_time + flush_period)
	{
	  last_time = cur_time;
	  fflush (ofile);
	}

      if (!debug_event)
	continue;

      if (pid)
	{
	  (void) cygwin_internal (CW_STRACE_TOGGLE, pid);
	  pid = 0;
	}

      switch (ev.dwDebugEventCode)
	{
	case CREATE_PROCESS_DEBUG_EVENT:
	  if (ev.u.CreateProcessInfo.hFile)
	    CloseHandle (ev.u.CreateProcessInfo.hFile);
	  add_child (ev.dwProcessId, ev.u.CreateProcessInfo.hProcess);
	  break;

	case CREATE_THREAD_DEBUG_EVENT:
	  break;

	case LOAD_DLL_DEBUG_EVENT:
	  if (ev.u.LoadDll.hFile)
	    CloseHandle (ev.u.LoadDll.hFile);
	  break;

	case OUTPUT_DEBUG_STRING_EVENT:
	  handle_output_debug_string (ev.dwProcessId,
				      ev.u.DebugString.lpDebugStringData,
				      mask, ofile);
	  break;

	case EXIT_PROCESS_DEBUG_EVENT:
	  res = ev.u.ExitProcess.dwExitCode >> 8;
	  remove_child (ev.dwProcessId);
	  break;
	case EXCEPTION_DEBUG_EVENT:
	  if (ev.u.Exception.ExceptionRecord.ExceptionCode != STATUS_BREAKPOINT)
	    {
	      status = DBG_EXCEPTION_NOT_HANDLED;
	      if (ev.u.Exception.dwFirstChance)
		fprintf (ofile, "--- Process %lu, exception %p at %p\n", ev.dwProcessId,
			 (void *) ev.u.Exception.ExceptionRecord.ExceptionCode,
			 ev.u.Exception.ExceptionRecord.ExceptionAddress);
	    }
	  break;
	}
      if (!ContinueDebugEvent (ev.dwProcessId, ev.dwThreadId, status))
	error (0, "couldn't continue debug event, windows error %d",
	       GetLastError ());
      if (!processes)
	break;
    }

  return res;
}

static void
dotoggle (pid_t pid)
{
  child_pid = (DWORD) cygwin_internal (CW_CYGWIN_PID_TO_WINPID, pid);
  if (!child_pid)
    {
      warn (0, "no such cygwin pid - %d", pid);
      child_pid = pid;
    }
  if (cygwin_internal (CW_STRACE_TOGGLE, child_pid))
    error (0, "failed to toggle tracing for process %d<%d>", pid, child_pid);

  return;
}

static DWORD
dostrace (unsigned mask, FILE *ofile, pid_t pid, char **argv)
{
  if (!pid)
    create_child (argv);
  else
    attach_process (pid);

  return proc_child (mask, ofile, pid);
}

typedef struct tag_mask_mnemonic
{
  unsigned long val;
  const char *text;
}
mask_mnemonic;

static const mask_mnemonic mnemonic_table[] = {
  {_STRACE_ALL, "all"},
  {_STRACE_FLUSH, "flush"},
  {_STRACE_INHERIT, "inherit"},
  {_STRACE_UHOH, "uhoh"},
  {_STRACE_SYSCALL, "syscall"},
  {_STRACE_STARTUP, "startup"},
  {_STRACE_DEBUG, "debug"},
  {_STRACE_PARANOID, "paranoid"},
  {_STRACE_TERMIOS, "termios"},
  {_STRACE_SELECT, "select"},
  {_STRACE_WM, "wm"},
  {_STRACE_SIGP, "sigp"},
  {_STRACE_MINIMAL, "minimal"},
  {_STRACE_EXITDUMP, "exitdump"},
  {_STRACE_SYSTEM, "system"},
  {_STRACE_NOMUTEX, "nomutex"},
  {_STRACE_MALLOC, "malloc"},
  {_STRACE_THREAD, "thread"},
  {0, NULL}
};

static unsigned long
mnemonic2ul (const char *nptr, char **endptr)
{
  // Look up mnemonic in table, return value.
  // *endptr = ptr to char that breaks match.
  const mask_mnemonic *mnp = mnemonic_table;

  while (mnp->text != NULL)
    {
      if (strcmp (mnp->text, nptr) == 0)
	{
	  // Found a match.
	  if (endptr != NULL)
	    {
	      *endptr = ((char *) nptr) + strlen (mnp->text);
	    }
	  return mnp->val;
	}
      mnp++;
    }

  // Didn't find it.
  if (endptr != NULL)
    {
      *endptr = (char *) nptr;
    }
  return 0;
}

static unsigned long
parse_mask (const char *ms, char **endptr)
{
  const char *p = ms;
  char *newp;
  unsigned long retval = 0, thisval;
  const size_t bufsize = 16;
  char buffer[bufsize];
  size_t len;

  while (*p != '\0')
    {
      // First extract the term, terminate it, and lowercase it.
      strncpy (buffer, p, bufsize);
      buffer[bufsize - 1] = '\0';
      len = strcspn (buffer, "+,\0");
      buffer[len] = '\0';
      strlwr (buffer);

      // Check if this is a mnemonic.  We have to do this first or strtoul()
      // will false-trigger on anything starting with "a" through "f".
      thisval = mnemonic2ul (buffer, &newp);
      if (buffer == newp)
	{
	  // This term isn't mnemonic, check if it's hex.
	  thisval = strtoul (buffer, &newp, 16);
	  if (newp != buffer + len)
	    {
	      // Not hex either, syntax error.
	      *endptr = (char *) p;
	      return 0;
	    }
	}

      p += len;
      retval += thisval;

      // Handle operators
      if (*p == '\0')
	break;
      if ((*p == '+') || (*p == ','))
	{
	  // For now these both equate to addition/ORing.  Until we get
	  // fancy and add things like "all-<something>", all we need do is
	  // continue the looping.
	  p++;
	  continue;
	}
      else
	{
	  // Syntax error
	  *endptr = (char *) p;
	  return 0;
	}
    }

  *endptr = (char *) p;
  return retval;
}

static void
usage (FILE *where = stderr)
{
  fprintf (where, "\
Usage: %s [OPTIONS] <command-line>\n\
Usage: %s [OPTIONS] -p <pid>\n\
Trace system calls and signals\n\
\n\
  -b, --buffer-size=SIZE       set size of output file buffer\n\
  -d, --no-delta               don't display the delta-t microsecond timestamp\n\
  -f, --trace-children         trace child processes (toggle - default true)\n\
  -h, --help                   output usage information and exit\n\
  -m, --mask=MASK              set message filter mask\n\
  -n, --crack-error-numbers    output descriptive text instead of error\n\
                               numbers for Windows errors\n\
  -o, --output=FILENAME        set output file to FILENAME\n\
  -p, --pid=n                  attach to executing program with cygwin pid n\n\
  -q, --quiet                  suppress messages about attaching, detaching, etc.\n\
  -S, --flush-period=PERIOD    flush buffered strace output every PERIOD secs\n\
  -t, --timestamp              use an absolute hh:mm:ss timestamp insted of \n\
                               the default microsecond timestamp.  Implies -d\n\
  -T, --toggle                 toggle tracing in a process already being\n\
                               traced. Requires -p <pid>\n\
  -u, --usecs                  toggle printing of microseconds timestamp\n\
  -v, --version                output version information and exit\n\
  -w, --new-window             spawn program under test in a new window\n\
\n", pgm, pgm);
  if ( where == stdout)
    fprintf (stdout, "\
    MASK can be any combination of the following mnemonics and/or hex values\n\
    (0x is optional).  Combine masks with '+' or ',' like so:\n\
\n\
                      --mask=wm+system,malloc+0x00800\n\
\n\
    Mnemonic Hex     Corresponding Def  Description\n\
    =========================================================================\n\
    all      0x00001 (_STRACE_ALL)      All strace messages.\n\
    flush    0x00002 (_STRACE_FLUSH)    Flush output buffer after each message.\n\
    inherit  0x00004 (_STRACE_INHERIT)  Children inherit mask from parent.\n\
    uhoh     0x00008 (_STRACE_UHOH)     Unusual or weird phenomenon.\n\
    syscall  0x00010 (_STRACE_SYSCALL)  System calls.\n\
    startup  0x00020 (_STRACE_STARTUP)  argc/envp printout at startup.\n\
    debug    0x00040 (_STRACE_DEBUG)    Info to help debugging. \n\
    paranoid 0x00080 (_STRACE_PARANOID) Paranoid info.\n\
    termios  0x00100 (_STRACE_TERMIOS)  Info for debugging termios stuff.\n\
    select   0x00200 (_STRACE_SELECT)   Info on ugly select internals.\n\
    wm       0x00400 (_STRACE_WM)       Trace Windows msgs (enable _strace_wm).\n\
    sigp     0x00800 (_STRACE_SIGP)     Trace signal and process handling.\n\
    minimal  0x01000 (_STRACE_MINIMAL)  Very minimal strace output.\n\
    exitdump 0x04000 (_STRACE_EXITDUMP) Dump strace cache on exit.\n\
    system   0x08000 (_STRACE_SYSTEM)   Serious error; goes to console and log.\n\
    nomutex  0x10000 (_STRACE_NOMUTEX)  Don't use mutex for synchronization.\n\
    malloc   0x20000 (_STRACE_MALLOC)   Trace malloc calls.\n\
    thread   0x40000 (_STRACE_THREAD)   Thread-locking calls.\n\
");
  if (where == stderr)
    fprintf (stderr, "Try '%s --help' for more information.\n", pgm);
  exit (where == stderr ? 1 : 0 );
}

struct option longopts[] = {
  {"buffer-size", required_argument, NULL, 'b'},
  {"help", no_argument, NULL, 'h'},
  {"flush-period", required_argument, NULL, 'S'},
  {"hex", no_argument, NULL, 'H'},
  {"mask", required_argument, NULL, 'm'},
  {"new-window", no_argument, NULL, 'w'},
  {"output", required_argument, NULL, 'o'},
  {"no-delta", no_argument, NULL, 'd'},
  {"pid", required_argument, NULL, 'p'},
  {"quiet", no_argument, NULL, 'q'},
  {"timestamp", no_argument, NULL, 't'},
  {"toggle", no_argument, NULL, 'T'},
  {"trace-children", no_argument, NULL, 'f'},
  {"translate-error-numbers", no_argument, NULL, 'n'},
  {"usecs", no_argument, NULL, 'u'},
  {"version", no_argument, NULL, 'v'},
  {NULL, 0, NULL, 0}
};

static const char *const opts = "+b:dhHfm:no:p:qS:tTuvw";

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
System Trace\n\
Copyright 2000, 2001, 2002, 2003, 2004, 2005 Red Hat, Inc.\n\
Compiled on %s\n\
", pgm, len, v, __DATE__);
}

int
main (int argc, char **argv)
{
  unsigned mask = 0;
  FILE *ofile = NULL;
  pid_t pid = 0;
  int opt;
  int toggle = 0;
  int sawquiet = -1;

  if (load_cygwin ())
    {
      char **av = (char **) cygwin_internal (CW_ARGV);
      if (av && (DWORD) av != (DWORD) -1)
	for (argc = 0, argv = av; *av; av++)
	  argc++;
    }

  if (!(pgm = strrchr (*argv, '\\')) && !(pgm = strrchr (*argv, '/')))
    pgm = *argv;
  else
    pgm++;

  while ((opt = getopt_long (argc, argv, opts, longopts, NULL)) != EOF)
    switch (opt)
      {
      case 'b':
	bufsize = atoi (optarg);
	break;
      case 'd':
	delta ^= 1;
	break;
      case 'f':
	forkdebug ^= 1;
	break;
      case 'h':
	// Print help and exit
	usage (stdout);
	break;
      case 'H':
	include_hex ^= 1;
	break;
      case 'm':
	{
	  char *endptr;
	  mask = parse_mask (optarg, &endptr);
	  if (*endptr != '\0')
	    {
	      // Bad mask expression.
	      error (0, "syntax error in mask expression \"%s\" near \
character #%d.\n", optarg, (int) (endptr - optarg), endptr);
	    }
	  break;
	}
      case 'n':
	numerror ^= 1;
	break;
      case 'o':
	if ((ofile = fopen (cygpath (optarg, NULL), "wb")) == NULL)
	  error (1, "can't open %s", optarg);
#ifdef F_SETFD
	(void) fcntl (fileno (ofile), F_SETFD, 0);
#endif
	break;
      case 'p':
	pid = strtoul (optarg, NULL, 10);
	strace_active |= 2;
	break;
      case 'q':
	if (sawquiet < 0)
	  sawquiet = 1;
	else
	  sawquiet ^= 1;
	break;
      case 'S':
	flush_period = strtoul (optarg, NULL, 10);
	break;
      case 't':
	hhmmss ^= 1;
	break;
      case 'T':
	toggle ^= 1;
	break;
      case 'u':
	// FIXME: currently unimplemented
	show_usecs ^= 1;
	delta ^= 1;
	break;
      case 'v':
	// Print version info and exit
	print_version ();
	return 0;
      case 'w':
	new_window ^= 1;
	break;
      case '?':
	fprintf (stderr, "Try '%s --help' for more information.\n", pgm);
	exit (1);
      }

  if (pid && argv[optind])
    error (0, "cannot provide both a command line and a process id");

  if (!pid && !argv[optind])
    error (0, "must provide either a command line or a process id");

  if (toggle && !pid)
    error (0, "must provide a process id to toggle tracing");

  if (!pid)
    quiet = sawquiet < 0 || !sawquiet;
  else if (sawquiet < 0)
    quiet = 0;
  else
    quiet = sawquiet;

  if (!mask)
    mask = _STRACE_ALL;

  if (bufsize)
    setvbuf (ofile, (char *) alloca (bufsize), _IOFBF, bufsize);

  if (!ofile)
    ofile = stdout;

  DWORD res = 0;
  if (toggle)
    dotoggle (pid);
  else
    res = dostrace (mask, ofile, pid, argv + optind);
  return res;
}

#undef CloseHandle

static BOOL
close_handle (HANDLE h, DWORD ok)
{
  child_list *c;
  for (c = &children; (c = c->next) != NULL;)
    if (c->hproc == h && c->id != ok)
      error (0, "Closing child handle %p", h);
  return CloseHandle (h);
}
