#include <stdio.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <signal.h>
#include "sys/strace.h"

/*  GCC runtime library's C++ EH code unfortunately pulls in stdio, and we
   get undefine references to __impure_ptr, and hence the following
   hack. It should be reasonably safe however as long as this file
   is built using -mno-cygwin as is intended.  */
int _impure_ptr;

/* we *know* we're being built with GCC */
#define alloca __builtin_alloca

static const char *pgm;
static int forkdebug = 0;
static int numerror = 1;
static int usecs = 1;
static int delta = 1;
static int hhmmss = 0;

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
    child_list () : id (0), hproc (NULL), saw_stars (0), nfields (0),
		    start_time (0), last_usecs (0), next (NULL) {}
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
  ExitProcess (1);
}

DWORD lastid = 0;
HANDLE lasth;

#define PROCFLAGS \
 PROCESS_ALL_ACCESS /*(PROCESS_DUP_HANDLE | PROCESS_TERMINATE | PROCESS_VM_READ | PROCESS_VM_WRITE)*/
static void
add_child (DWORD id, HANDLE hproc)
{
  child_list *c = children.next;
  children.next = new (child_list);
  children.next->next = c;
  lastid = children.next->id = id;
  HANDLE me = GetCurrentProcess ();
  lasth = children.next->hproc = hproc;
}

static child_list *
get_child (DWORD id)
{
  child_list *c;
  for (c = &children; (c = c->next) != NULL; )
    if (c->id == id)
      return c;

  error (0, "no process id %d found", id);
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
	delete c1;
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
  ~linebuf () {if (buf) free (buf);}
  void add (const char *what, int len);
  void add (const char *what) {add (what, strlen (what));}
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
make_command_line (linebuf& one_line, char **argv)
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

static void
create_child (char **argv)
{
  linebuf one_line;

  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  BOOL ret;
  DWORD flags;

  if (!*argv)
    error (0, "no program argument specified");

  memset (&si, 0, sizeof (si));
  si.cb = sizeof (si);

  /* cygwin32_conv_to_win32_path (exec_file, real_path);*/

  flags = forkdebug ? 0 : DEBUG_ONLY_THIS_PROCESS;
  flags |= /*CREATE_NEW_PROCESS_GROUP | */CREATE_DEFAULT_ERROR_MODE | DEBUG_PROCESS;

  make_command_line (one_line, argv);

  SetConsoleCtrlHandler (NULL, 0);
  ret = CreateProcess (0,
		       one_line.buf,/* command line */
		       NULL,	/* Security */
		       NULL,	/* thread */
		       TRUE,	/* inherit handles */
		       flags,	/* start flags */
		       NULL,
		       NULL,	/* current directory */
		       &si,
		       &pi);
  if (!ret)
    error (0, "error creating process %s, (error %d)", *argv, GetLastError());

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
		      (LPTSTR) buf,
		      sizeof (buf),
		      NULL))
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
  long long now = t + ((long long) usecs * 10);
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
  HANDLE hchild = child->hproc;
  #define INTROLEN (sizeof (alen) - 1)

  if (id == lastid && hchild != lasth)
    warn (0, "%p != %p", hchild, lasth);

  alen[INTROLEN] = '\0';
  if (!ReadProcessMemory (hchild, p, alen, INTROLEN, &nbytes))
#ifndef DEBUGGING
    return;
#else
    error (0, "couldn't get message length from subprocess %d<%p>, windows error %d",
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
      if (special == _STRACE_INTERFACE_ACTIVATE_ADDR)
	len = 17;
    }

  char *buf;
  buf = (char *) alloca (len + 65) + 10;

  if (!ReadProcessMemory (hchild, ((char *) p) + INTROLEN, buf, len, &nbytes))
    error (0, "couldn't get message from subprocess, windows error %d",
	   GetLastError ());

  buf[len] = '\0';
  char *s = strtok (buf, " ");

  unsigned n = strtoul (s, NULL, 16);

  s = strchr (s, '\0') + 1;

  if (special == _STRACE_INTERFACE_ACTIVATE_ADDR)
    {
      DWORD new_flag = 1;
      if (!WriteProcessMemory (hchild, (LPVOID) n, &new_flag,
			      sizeof (new_flag), &nbytes))
	error (0, "couldn't write strace flag to subprocess, windows error %d",
	       GetLastError ());
      return;
    }

  char *origs = s;

  if (mask & n)
    /* got it */;
  else if (!(mask & _STRACE_ALL) || (n & _STRACE_NOTALL))
    return;		/* This should not be included in "all" output */

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
      ptusec = s;
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
	      break; 	// Should never happen
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
		  fprintf (ofile, "Date/Time:    %d-%02d-%02d %02d:%02d:%02d\n",
			   st->wYear, st->wMonth, st->wDay, st->wHour, st->wMinute, st->wSecond);
		  child->saw_stars++;
		}
	    }
	}
    }

  long long d = usecs - child->last_usecs;
  char intbuf[40];

  if (child->saw_stars < 2 || s != origs)
    /* Nothing */;
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

  child->last_usecs = usecs;
  if (numerror || !output_winerror (ofile, s))
    fputs (s, ofile);
  fflush (ofile);
}

static void
proc_child (unsigned mask, FILE *ofile)
{
  DEBUG_EVENT ev;
  int processes = 0;
  SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_HIGHEST);
  while (1)
    {
      BOOL debug_event = WaitForDebugEvent (&ev, 1000);
      if (!debug_event)
	continue;

      switch (ev.dwDebugEventCode)
	{
	case CREATE_PROCESS_DEBUG_EVENT:
	  if (ev.u.CreateProcessInfo.hFile)
	    CloseHandle (ev.u.CreateProcessInfo.hFile);
	  add_child (ev.dwProcessId, ev.u.CreateProcessInfo.hProcess);
	  processes++;
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
	  remove_child (ev.dwProcessId);
	  break;
	}
      if (!ContinueDebugEvent (ev.dwProcessId, ev.dwThreadId,
				DBG_CONTINUE))
	error (0, "couldn't continue debug event, windows error %d",
	       GetLastError ());
      if (ev.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT && --processes == 0)
	break;
    }
}

static void
dostrace (unsigned mask, FILE *ofile, char **argv)
{
  create_child (argv);
  proc_child (mask, ofile);

  return;
}

int
main(int argc, char **argv)
{
  unsigned mask = 0;
  FILE *ofile = NULL;
  int opt;

  if (!(pgm = strrchr (*argv, '\\')) && !(pgm = strrchr (*argv, '/')))
    pgm = *argv;
  else
    pgm++;

  while ((opt = getopt (argc, argv, "m:o:fndut")) != EOF)
    switch (opt)
      {
      case 'f':
	forkdebug ^= 1;
	break;
      case 'm':
	mask = strtoul (optarg, NULL, 16);
	break;
      case 'o':
	if ((ofile = fopen (optarg, "w")) == NULL)
	  error (1, "can't open %s", optarg);
#ifdef F_SETFD
	(void) fcntl (fileno (ofile), F_SETFD, 0);
#endif
	break;
      case 'n':
	numerror ^= 1;
	break;
      case 't':
	hhmmss ^= 1;
	break;
      case 'd':
	delta ^= 1;
	break;
      case 'u':
	usecs ^= 1;
      }

  if (!mask)
    mask = 1;

  if (!ofile)
    ofile = stdout;

  dostrace (mask, ofile, argv + optind);
}

#undef CloseHandle

static BOOL
close_handle (HANDLE h, DWORD ok)
{
  child_list *c;
  for (c = &children; (c = c->next) != NULL; )
    if (c->hproc == h && c->id != ok)
      error (0, "Closing child handle %p", h);
  return CloseHandle (h);
}
