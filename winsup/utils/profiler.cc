/*
    profiler.cc
    Periodically samples IP of a process and its DLLs; writes gprof data files.

    Written by Mark Geisert <mark@maxrnd.com>, who admits to
    copying pretty liberally from strace.cc.  h/t to cgf for strace!

    This file is part of Cygwin.

    This software is a copyrighted work licensed under the terms of the
    Cygwin license.  Please consult the file "CYGWIN_LICENSE" for details.
*/

#define WIN32_LEAN_AND_MEAN
#include <winternl.h>

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <io.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/cygwin.h>
#include "cygwin/version.h"
#include "gcc_seh.h"
typedef unsigned long ulong;
typedef unsigned short ushort;
typedef uint16_t u_int16_t; // Non-standard sized type needed by ancient gmon.h
#define NO_GLOBALS_H
#include "gmon.h"
#include "path.h"

/* Undo this #define from winsup.h. */
#ifdef ExitThread
#undef ExitThread
#endif

#define SCALE_SHIFT 2 // == 4 bytes of address space per bucket
#define MS_VC_EXCEPTION 0x406D1388 // thread name notification from child

DWORD       child_pid;
int         debugging = 0;
void       *drive_map;
int         events = 0;
int         forkprofile = 0;
int         new_window;
int         numprocesses;
FILE       *ofile = stdout;
const char *pgm;
char       *prefix = (char *) "gmon.out";
int         samplerate = 100; // in Hz; up to 1000 might work
int         verbose = 0;

void __attribute__ ((__noreturn__))
usage (FILE *where = stderr)
{
  fprintf (where, "\
Usage: %s [OPTIONS] <command-line>\n\
   or: %s [OPTIONS] -p <pid>\n\
\n\
Profiles a command or process by sampling its IP (instruction pointer).\n\
OPTIONS are:\n\
\n\
  -d, --debug            Display debugging messages (toggle: default false)\n\
  -e, --events           Display Windows DEBUG_EVENTS (toggle: default false)\n\
  -f, --fork-profile     Profile child processes (toggle: default false)\n\
  -h, --help             Display usage information and exit\n\
  -o, --output=FILENAME  Write output to file FILENAME rather than stdout\n\
  -p, --pid=N            Attach to running program with Cygwin pid N\n\
                         ...                    or with Windows pid -N\n\
  -s, --sample-rate=N    Set IP sampling rate to N Hz (default 100)\n\
  -v, --verbose          Display more status messages (toggle: default false)\n\
  -V, --version          Display version information and exit\n\
  -w, --new-window       Launch given command in a new window\n\
\n", pgm, pgm);

  exit (where == stderr ? 1 : 0 );
}

/* A span is a memory address range covering an EXE's or DLL's .text segment. */
struct span_list
{
  WCHAR  *name;
  LPVOID  base;
  size_t  textlo;
  size_t  texthi;
  int     hitcount;
  int     hitbuckets;
  int     numbuckets;
  int    *buckets;
  struct span_list *next;
};

/* A thread. */
struct thread_list
{
  DWORD   tid;
  HANDLE  hthread;
  WCHAR  *name;
  struct thread_list *next;
};

/* A child is any process being sampled in this profiler run. */
struct child_list
{
  DWORD  pid;
  volatile int profiling;
  HANDLE hproc;
  HANDLE hquitevt;
  HANDLE hprofthr;
  CONTEXT            *context;
  struct thread_list *threads;
  struct span_list   *spans;
  struct child_list  *next;
};

child_list children;
typedef struct child_list child;

void
note (const char *fmt, ...)
{
  va_list args;
  char    buf[4096];

  va_start (args, fmt);
  vsprintf (buf, fmt, args);
  va_end (args);

  fputs (buf, ofile);
  fflush (ofile);
}

void
warn (int geterrno, const char *fmt, ...)
{
  va_list args;
  char    buf[4096];

  va_start (args, fmt);
  sprintf (buf, "%s: ", pgm);
  vsprintf (strchr (buf, '\0'), fmt, args);
  va_end (args);
  if (geterrno)
    perror (buf);
  else
    {
      fputs (buf, ofile);
      fputs ("\n", ofile);
      fflush (ofile);
    }
}

void __attribute__ ((noreturn))
error (int geterrno, const char *fmt, ...)
{
  va_list args;

  va_start (args, fmt);
  warn (geterrno, fmt, args);
  va_end (args);

  exit (1);
}

size_t
sample (CONTEXT *context, HANDLE h)
{
  size_t status;

  if (-1U == SuspendThread (h))
    return 0ULL;
  status = GetThreadContext (h, context);
  if (-1U == ResumeThread (h))
    if (verbose)
      note ("*** unable to resume thread %d; continuing anyway\n", h);

  if (0 == status)
    {
      if (verbose)
        note ("*** unable to get context for thread %d\n", h);
      return 0ULL;
    }
  else
//TODO this approach does not support 32-bit executables on 64-bit
#ifdef __x86_64__
    return context->Rip;
#else
    return context->Eip;
#endif
}

void
bump_bucket (child *c, size_t pc)
{
  span_list *s = c->spans;

//note ("%lu %p\n", (ulong) c->pid, pc);
  if (pc == 0ULL)
    return;
  while (s)
    {
      if (pc >= s->textlo && pc < s->texthi)
        {
          if (0 == s->buckets[(pc - s->textlo) >> SCALE_SHIFT]++)
            ++s->hitbuckets;
          ++s->hitcount;
          return;
        }
      s = s->next;
    }

  /*TODO If the child has dynamically created an executable memory region, we
   *     won't notice it until the profiler thread happens to sample an
   *     instruction in that region.  We could then add a new span to record
   *     hits on this new region. (QueryVirtualMemory to obtain limits?)
   *
   *     Note that if the app dynamically adds and deletes such regions, the
   *     profiling info on them will be confusing if their addresses overlap.
   */
  if (verbose)
    note ("*** pc %p out of range for pid %lu\n", pc, (ulong) c->pid);
}

/* profiler runs on its own thread; each child has a separate profiler. */
DWORD WINAPI
profiler (void *vp)
{
  child *c = (child *) vp;

  while (c->profiling)
    {
      thread_list *t = c->threads;

      while (t)
        {
          if (t->hthread)
            bump_bucket (c, sample (c->context, t->hthread));
          t = t->next;
        }

      if (WaitForSingleObject (c->hquitevt, 1000 / samplerate) == WAIT_OBJECT_0)
        break;
    }

  return 0;
}

void
start_profiler (child *c)
{
  DWORD  tid;

  if (verbose)
    note ("*** start profiler thread on pid %lu\n", (ulong) c->pid);
  c->context = (CONTEXT *) calloc (1, sizeof (CONTEXT));
  if (!c->context)
    error (0, "unable to allocate CONTEXT buffer\n");
  c->context->ContextFlags = CONTEXT_CONTROL;
  c->hquitevt = CreateEvent (NULL, TRUE, FALSE, NULL);
  if (!c->hquitevt)
    error (0, "unable to create quit event\n");
  c->profiling = 1;
  c->hprofthr = CreateThread (NULL, 0, profiler, (void *) c, 0, &tid);
  if (!c->hprofthr)
    error (0, "unable to create profiling thread\n");

  /* There is a temptation to raise the execution priority of the profiling
   * threads.  Don't do this, or at least don't do it this way.  Testing
   * showed that it was possible to starve system processes which makes the
   * system unresponsive.  Raising prio doesn't seem to be needed at all.
   *
  SetThreadPriority (c->hprofthr, THREAD_PRIORITY_TIME_CRITICAL);
   */
}

void
stop_profiler (child *c)
{
  if (verbose)
    note ("*** stop profiler thread on pid %lu\n", (ulong) c->pid);
  c->profiling = 0;
  SignalObjectAndWait (c->hquitevt, c->hprofthr, INFINITE, FALSE);
  CloseHandle (c->hquitevt);
  CloseHandle (c->hprofthr);
  c->hquitevt = c->hprofthr = 0;
}

/* Create a gmon.out file for each EXE or DLL that has at least one sample. */
void
dump_profile_data (child *c)
{
  int        fd;
  char       filename[MAX_PATH + 1];
  struct gmonhdr hdr;
  span_list *s = c->spans;

  while (s)
    {
      if (s->hitbuckets == 0)
        {
          s = s->next;
          continue;
        }

      if (s->name)
        {
          WCHAR *name = 1 + wcsrchr (s->name, L'\\');
          sprintf (filename, "%s.%lu.%ls", prefix, (ulong) c->pid, name);
        }
      else
        sprintf (filename, "%s.%lu", prefix, (ulong) c->pid);

      fd = open (filename, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY);
      if (fd < 0)
        error (0, "dump_profile_data: unable to create %s\n", filename);

      memset (&hdr, 0, sizeof (hdr));
      hdr.lpc = s->textlo;
      hdr.hpc = s->texthi;
      hdr.ncnt = s->numbuckets * sizeof (short) + sizeof (hdr);
      hdr.version = GMONVERSION;
      hdr.profrate = samplerate;

      /* Our buckets hold more than gmon standard buckets, so truncate here. */
      ushort *gmonbuckets = (ushort *) calloc (s->numbuckets, sizeof (ushort));
      for (int i = 0; i < s->numbuckets; i++)
        {
          if (s->buckets[i])
            {
              if (s->buckets[i] > 65535)
                {
                  note ("  WARNING: bucket %d: value %d truncated to %d\n",
                        i, s->buckets[i], 65535);
                  gmonbuckets[i] = 65535;
                }
              else
                gmonbuckets[i] = s->buckets[i];
            }
        }

      write (fd, &hdr, sizeof (hdr));
      write (fd, gmonbuckets, hdr.ncnt - sizeof (hdr));
      note ("%d %s across %d %s written to %s\n", s->hitcount,
            s->hitcount == 1 ? "sample" : "samples", s->hitbuckets,
            s->hitbuckets == 1 ? "bucket" : "buckets", filename);
      close (fd);
      chmod (filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
      free (gmonbuckets);

      s = s->next;
    }
}

HANDLE lasth;
DWORD  lastpid = 0;

child *
get_child (DWORD pid)
{
  child *c;

  for (c = &children; (c = c->next) != NULL;)
    if (c->pid == pid)
      return (child *) c;

  return NULL;
}

void add_span (DWORD, WCHAR *, LPVOID, HANDLE);

void
add_child (DWORD pid, WCHAR *name, LPVOID base, HANDLE hproc)
{
  if (!get_child (pid))
    {
      child *c = children.next;
      children.next = (child *) calloc (1, sizeof (child));
      children.next->next = c;
      lastpid = children.next->pid = pid;
      lasth = children.next->hproc = hproc;
      add_span (pid, name, base, hproc);
      start_profiler (children.next);
      numprocesses++;
      if (verbose)
        note ("*** Windows process %lu attached\n", (ulong) pid);
    }
}

void
remove_child (DWORD pid)
{
  child *c;

  if (pid == lastpid)
    lastpid = 0;
  for (c = &children; c->next != NULL; c = c->next)
    if (c->next->pid == pid)
      {
        child *c1 = c->next;
        c->next = c1->next;
        stop_profiler (c1);
        dump_profile_data (c1);
        CloseHandle (c1->hproc);
        c1->hproc = 0;
        free (c1);
        if (verbose)
          note ("*** Windows process %lu detached\n", (ulong) pid);
        numprocesses--;
        return;
      }

  error (0, "no process id %d found", pid);
}

void
add_thread (DWORD pid, DWORD tid, HANDLE h, WCHAR *name)
{
  child *c = get_child (pid);

  if (!c)
    error (0, "add_thread: pid %lu not found\n", (ulong) pid);

  thread_list *t = (thread_list *) calloc (1, sizeof (thread_list));
  t->tid = tid;
  t->hthread = h;
  t->name = name;

  t->next = c->threads;
  c->threads = t;
}

void
remove_thread (DWORD pid, DWORD tid)
{
  child *c = get_child (pid);

  if (!c)
    error (0, "remove_thread: pid %lu not found\n", (ulong) pid);

  thread_list *t = c->threads;
  while (t)
    {
      if (t->tid == tid)
        {
          /*TODO We don't free(t), we just zero it out.  Maybe revisit this. */
          t->tid = 0;
          CloseHandle (t->hthread);
          t->hthread = 0;
          if (t->name)
            free (t->name);
          t->name = NULL;
          return;
        }
      t = t->next;
    }

  error (0, "remove_thread: pid %lu tid %lu not found\n",
             (ulong) pid, (ulong) tid);
}

void
read_child (void *buf, SIZE_T size, void *addr, HANDLE h)
{
  SIZE_T len;

  if (debugging)
    note ("read %d bytes at %p from handle %d\n", size, addr, h);
  if (0 == ReadProcessMemory (h, addr, buf, size, &len))
    error (0, "read_child: failed\n");
  if (len != size)
    error (0, "read_child: asked for %d bytes but got %d\n", size, len);
}

IMAGE_SECTION_HEADER *
find_text_section (LPVOID base, HANDLE h)
{
  static IMAGE_SECTION_HEADER asect;
  DWORD  lfanew;
  WORD   machine;
  WORD   nsects;
  DWORD  ntsig;
  char  *ptr = (char *) base;

  IMAGE_DOS_HEADER *idh = (IMAGE_DOS_HEADER *) ptr;
  read_child ((void *) &lfanew, sizeof (lfanew), &idh->e_lfanew, h);
  ptr += lfanew;

  /* Code handles 32- or 64-bit headers depending on compilation environment. */
  /*TODO It doesn't yet handle 32-bit headers on 64-bit Cygwin or v/v.        */
  IMAGE_NT_HEADERS *inth = (IMAGE_NT_HEADERS *) ptr;
  read_child ((void *) &ntsig, sizeof (ntsig), &inth->Signature, h);
  if (ntsig != IMAGE_NT_SIGNATURE)
    error (0, "find_text_section: NT signature not found\n");

  read_child ((void *) &machine, sizeof (machine),
              &inth->FileHeader.Machine, h);
#ifdef __x86_64__
  if (machine != IMAGE_FILE_MACHINE_AMD64)
#else
  if (machine != IMAGE_FILE_MACHINE_I386)
#endif
    error (0, "target program was built for different machine architecture\n");

  read_child ((void *) &nsects, sizeof (nsects),
              &inth->FileHeader.NumberOfSections, h);
  ptr += sizeof (*inth);

  IMAGE_SECTION_HEADER *ish = (IMAGE_SECTION_HEADER *) ptr;
  for (int i = 0; i < nsects; i++)
    {
      read_child ((void *) &asect, sizeof (asect), ish, h);
      if (0 == memcmp (".text\0\0\0", &asect.Name, 8))
        return &asect;
      ish++;
    }

  error (0, ".text section not found\n");
}

//TODO Extend add_span to add all executable sections of this exe/dll
void
add_span (DWORD pid, WCHAR *name, LPVOID base, HANDLE h)
{
  child *c = get_child (pid);

  if (!c)
    error (0, "add_span: pid %lu not found\n", (ulong) pid);

  IMAGE_SECTION_HEADER *sect = find_text_section (base, c->hproc);
  span_list *s = (span_list *) calloc (1, sizeof (span_list));
  s->name = name;
  s->base = base;
  s->textlo = sect->VirtualAddress + (size_t) base;
  s->texthi = s->textlo + sect->Misc.VirtualSize;
  s->numbuckets = (s->texthi - s->textlo) >> SCALE_SHIFT;
  s->buckets = (int *) calloc (s->numbuckets, sizeof (int));
  if (debugging)
    note ("    span %p - %p, size %X, numbuckets %d\n",
          s->textlo, s->texthi, s->texthi - s->textlo, s->numbuckets);

  s->next = c->spans;
  c->spans = s;
}

#define LINE_BUF_CHUNK 128

class linebuf
{
  size_t  alloc;
public:
  size_t  ix;
  char   *buf;
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
  int    buflen;
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

void
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

BOOL WINAPI
ctrl_c (DWORD)
{
  static int tic = 1;

  if ((tic ^= 1) && !GenerateConsoleCtrlEvent (CTRL_C_EVENT, 0))
    error (0, "couldn't send CTRL-C to child, win32 error %d\n",
           GetLastError ());
  return TRUE;
}

WCHAR cygwin_dll_path[32768]; // required by path.cc helper funcs used herein

#define DEBUG_PROCESS_DETACH_ON_EXIT    0x00000001
#define DEBUG_PROCESS_ONLY_THIS_PROCESS 0x00000002

void
attach_process (pid_t pid)
{
  child_pid = pid < 0 ? (DWORD) -pid :
        (DWORD) cygwin_internal (CW_CYGWIN_PID_TO_WINPID, pid);

  if (!DebugActiveProcess (child_pid))
    error (0, "couldn't attach to pid %d for debugging", child_pid);

  if (forkprofile)
    {
      HANDLE h = OpenProcess (PROCESS_ALL_ACCESS, FALSE, child_pid);

      if (h)
        {
          /* Try to turn off DEBUG_ONLY_THIS_PROCESS so we can follow forks. */
          ULONG DebugFlags = DEBUG_PROCESS_DETACH_ON_EXIT;
          NTSTATUS status = NtSetInformationProcess (h, ProcessDebugFlags,
                                        &DebugFlags, sizeof (DebugFlags));
          if (!NT_SUCCESS (status))
            warn (0, "Could not clear DEBUG_ONLY_THIS_PROCESS (%x), "
                  "will not trace child processes", status);

          CloseHandle (h);
        }
    }

  return;
}

void
create_child (char **argv)
{
  DWORD               flags;
  linebuf             one_line;
  PROCESS_INFORMATION pi;
  BOOL                ret;
  STARTUPINFO         si;

  if (strchr (*argv, '/'))
      *argv = cygpath (*argv, NULL);
  memset (&si, 0, sizeof (si));
  si.cb = sizeof (si);

  flags = CREATE_DEFAULT_ERROR_MODE
          | (forkprofile ? DEBUG_PROCESS : DEBUG_ONLY_THIS_PROCESS);
  if (new_window)
    flags |= CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP;

  make_command_line (one_line, argv);

  SetConsoleCtrlHandler (NULL, 0);

  const char *cygwin_env = getenv ("CYGWIN");
  const char *space;

  if (cygwin_env && strlen (cygwin_env) <= 256) /* sanity check */
    space = " ";
  else
    space = cygwin_env = "";

  char *newenv = (char *) malloc (sizeof ("CYGWIN=noglob") +
                                  strlen (space) + strlen (cygwin_env));
  sprintf (newenv, "CYGWIN=noglob%s%s", space, cygwin_env);
  putenv (newenv);
  ret = CreateProcess (0, one_line.buf, /* command line */
                       NULL,    /* Security */
                       NULL,    /* thread */
                       TRUE,    /* inherit handles */
                       flags,   /* start flags */
                       NULL,    /* default environment */
                       NULL,    /* current directory */
                       &si, &pi);
  if (!ret)
    error (0, "error creating process %s, (error %d)", *argv,
           GetLastError ());

  CloseHandle (pi.hThread);
  CloseHandle (pi.hProcess);
  child_pid = pi.dwProcessId;
  SetConsoleCtrlHandler (ctrl_c, 1);
}

void
handle_output_debug_string (DWORD pid, OUTPUT_DEBUG_STRING_INFO *ev)
{
  char  *buf = (char *) alloca (ev->nDebugStringLength);
  child *c = get_child (pid);

  if (!c)
    error (0, "handle_output_debug_string: pid %lu not found\n", (ulong) pid);

  read_child (buf, ev->nDebugStringLength, ev->lpDebugStringData, c->hproc);
  if (strncmp (buf, "cYg", 3))
    { // string is not from Cygwin, it's from the target app; just display it
      if (ev->fUnicode)
        note ("%ls", buf);
      else
        note ("%s", buf);
    }
  //else TODO Possibly decode and display Cygwin-internal debug string
}

BOOL
GetFileNameFromHandle (HANDLE hFile, WCHAR pszFilename[MAX_PATH+1])
{
  BOOL     result = FALSE;
  ULONG    len = 0;
  OBJECT_NAME_INFORMATION *ntfn = (OBJECT_NAME_INFORMATION *) alloca (65536);
  NTSTATUS status = NtQueryObject (hFile, ObjectNameInformation,
                                   ntfn, 65536, &len);
  if (NT_SUCCESS (status))
    {
      PWCHAR win32path = ntfn->Name.Buffer;
      win32path[ntfn->Name.Length / sizeof (WCHAR)] = L'\0';

      /* NtQueryObject returns a native NT path.  (Try to) convert to Win32. */
      if (drive_map)
        win32path = (PWCHAR) cygwin_internal (CW_MAP_DRIVE_MAP, drive_map,
                                              win32path);
      pszFilename[0] = L'\0';
      wcsncat (pszFilename, win32path, MAX_PATH);
      result = TRUE;
    }

  return result;
}

char *
cygwin_pid (DWORD winpid)
{
  static char  buf[48];
  DWORD        cygpid;
  static DWORD max_cygpid = 0;

  if (!max_cygpid)
    max_cygpid = (DWORD) cygwin_internal (CW_MAX_CYGWIN_PID);

  cygpid = (DWORD) cygwin_internal (CW_WINPID_TO_CYGWIN_PID, winpid);

  if (cygpid >= max_cygpid)
    snprintf (buf, sizeof (buf), "%lu", (ulong) winpid);
  else
    snprintf (buf, sizeof (buf), "%lu (pid: %lu)",
              (ulong) winpid, (ulong) cygpid);
  return buf;
}

DWORD
profile1 (FILE *ofile, pid_t pid)
{
  DEBUG_EVENT ev;
  DWORD       res = 0;

  SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_HIGHEST);
  while (1)
    {
      BOOL debug_event = WaitForDebugEvent (&ev, INFINITE);
      DWORD status = DBG_CONTINUE;

      if (!debug_event)
        continue;

      /* Usually continue event here so child resumes while we process event. */
      if (ev.dwDebugEventCode != EXCEPTION_DEBUG_EVENT &&
          ev.dwDebugEventCode != OUTPUT_DEBUG_STRING_EVENT)
        debug_event = ContinueDebugEvent (ev.dwProcessId, ev.dwThreadId, status);

      switch (ev.dwDebugEventCode)
        {
        case CREATE_PROCESS_DEBUG_EVENT:
          WCHAR exename[MAX_PATH+1];

          if (!GetFileNameFromHandle (ev.u.CreateProcessInfo.hFile, exename))
            wcscpy (exename, L"(unknown)");
          if (events)
            {
              note ("--- Process %s created from %ls\n",
                    cygwin_pid (ev.dwProcessId), exename);
              note ("--- Process %s thread %lu created at %p\n",
                    cygwin_pid (ev.dwProcessId), ev.dwThreadId,
                    ev.u.CreateProcessInfo.lpStartAddress);
            }
          if (ev.u.CreateProcessInfo.hFile)
            CloseHandle (ev.u.CreateProcessInfo.hFile);
          add_child (ev.dwProcessId, wcsdup (exename),
                     ev.u.CreateProcessInfo.lpBaseOfImage,
                     ev.u.CreateProcessInfo.hProcess);
          add_thread (ev.dwProcessId, ev.dwThreadId,
                      ev.u.CreateProcessInfo.hThread, wcsdup (exename));
          break;

        case CREATE_THREAD_DEBUG_EVENT:
          if (events)
            note ("--- Process %s thread %lu created at %p\n",
                  cygwin_pid (ev.dwProcessId), ev.dwThreadId,
                  ev.u.CreateThread.lpStartAddress);
          add_thread (ev.dwProcessId, ev.dwThreadId,
                      ev.u.CreateThread.hThread, NULL);
          break;

        case LOAD_DLL_DEBUG_EVENT:
          WCHAR dllname[MAX_PATH+1];

          /* lpImageName is not always populated, so find the filename for
             hFile instead. */
          if (!GetFileNameFromHandle (ev.u.LoadDll.hFile, dllname))
            wcscpy (dllname, L"(unknown)");

          if (events)
              note ("--- Process %s loaded %ls at %p\n",
                    cygwin_pid (ev.dwProcessId), dllname,
                    ev.u.LoadDll.lpBaseOfDll);
          add_span (ev.dwProcessId, wcsdup (dllname),
                    ev.u.LoadDll.lpBaseOfDll, ev.u.LoadDll.hFile);

          if (ev.u.LoadDll.hFile)
            CloseHandle (ev.u.LoadDll.hFile);
          break;

        case UNLOAD_DLL_DEBUG_EVENT:
          if (events)
            note ("--- Process %s unloaded DLL at %p\n",
                  cygwin_pid (ev.dwProcessId), ev.u.UnloadDll.lpBaseOfDll);
          break;

        case OUTPUT_DEBUG_STRING_EVENT:
          handle_output_debug_string (ev.dwProcessId, &ev.u.DebugString);
          status = DBG_EXCEPTION_HANDLED;
          debug_event = ContinueDebugEvent (ev.dwProcessId,
                                            ev.dwThreadId, status);
          break;

        case EXIT_PROCESS_DEBUG_EVENT:
          if (events)
            note ("--- Process %s exited with status 0x%lx\n",
                  cygwin_pid (ev.dwProcessId), ev.u.ExitProcess.dwExitCode);
          res = ev.u.ExitProcess.dwExitCode;
          remove_child (ev.dwProcessId);
          break;

        case EXIT_THREAD_DEBUG_EVENT:
          if (events)
            note ("--- Process %s thread %lu exited with status 0x%lx\n",
                  cygwin_pid (ev.dwProcessId), ev.dwThreadId,
                  ev.u.ExitThread.dwExitCode);
          remove_thread (ev.dwProcessId, ev.dwThreadId);
          break;

        case EXCEPTION_DEBUG_EVENT:
          status = DBG_EXCEPTION_HANDLED;
          switch (ev.u.Exception.ExceptionRecord.ExceptionCode)
            {
            case MS_VC_EXCEPTION:
              //TODO Decode exception info to get thread name; set it internally
              // fall thru

            case STATUS_BREAKPOINT:
              break;

#ifdef __x86_64__
            case STATUS_GCC_THROW:
            case STATUS_GCC_UNWIND:
            case STATUS_GCC_FORCED:
              status = DBG_EXCEPTION_NOT_HANDLED;
              break;
#endif

            default:
              status = DBG_EXCEPTION_NOT_HANDLED;
              if (ev.u.Exception.dwFirstChance)
                note ("--- Process %s thread %lu exception %08x at %p\n",
                      cygwin_pid (ev.dwProcessId), ev.dwThreadId,
                      ev.u.Exception.ExceptionRecord.ExceptionCode,
                      ev.u.Exception.ExceptionRecord.ExceptionAddress);
              break;
            }
          debug_event = ContinueDebugEvent (ev.dwProcessId,
                                            ev.dwThreadId, status);
          break;
        }

      if (!debug_event)
        error (0, "couldn't continue debug event, windows error %d",
               GetLastError ());
      if (!numprocesses)
        break;
    }

  return res;
}

DWORD
doprofile (FILE *ofile, pid_t pid, char **argv)
{
  if (pid)
    attach_process (pid);
  else
    create_child (argv);

  return profile1 (ofile, pid);
}

struct option longopts[] = {
  {"debug",       no_argument,       NULL, 'd'},
  {"events",      no_argument,       NULL, 'e'},
  {"help",        no_argument,       NULL, 'h'},
  {"new-window",  no_argument,       NULL, 'w'},
  {"output",      required_argument, NULL, 'o'},
  {"pid",         required_argument, NULL, 'p'},
  {"fork-profile",no_argument,       NULL, 'f'},
  {"sample-rate", required_argument, NULL, 's'},
  {"verbose",     no_argument,       NULL, 'v'},
  {"version",     no_argument,       NULL, 'V'},
  {NULL,          0,                 NULL, 0  }
};

const char *const opts = "+dehfo:p:s:vVw";

void __attribute__ ((__noreturn__))
print_version ()
{
  char *year_of_build = strrchr (__DATE__, ' ') + 1;
  printf ("profiler (cygwin) %d.%d.%d\n"
          "IP-Sampling Profiler\n"
          "Copyright (C) %s%s Cygwin Authors\n"
          "This is free software; see the source for copying conditions.  "
          "There is NO\nwarranty; not even for MERCHANTABILITY or FITNESS "
          "FOR A PARTICULAR PURPOSE.\n",
          CYGWIN_VERSION_DLL_MAJOR / 1000,
          CYGWIN_VERSION_DLL_MAJOR % 1000,
          CYGWIN_VERSION_DLL_MINOR,
          strncmp (year_of_build, "2021", 4) ? "2021 - " : "",
          year_of_build);
  exit (0);
}

int
main (int argc, char **argv)
{
  int    opt;
  pid_t  pid = 0;
  char  *ptr;
  DWORD  ret = 0;

  _setmode (1, O_BINARY);
  _setmode (2, O_BINARY);

  if (!(pgm = strrchr (*argv, '\\')) && !(pgm = strrchr (*argv, '/')))
    pgm = *argv;
  else
    pgm++;

  while ((opt = getopt_long (argc, argv, opts, longopts, NULL)) != EOF)
    switch (opt)
      {
      case 'd':
        debugging ^= 1;
        if (debugging)
          verbose = events = 1; // debugging turns these on too
        break;

      case 'e':
        events ^= 1;
        events |= debugging; // debugging turns on events too
        break;

      case 'f':
        forkprofile ^= 1;
        break;

      case 'h':
        /* Print help and exit. */
        usage (ofile);

      case 'o':
        if ((ofile = fopen (cygpath (optarg, NULL), "wb")) == NULL)
          error (1, "can't open %s", optarg);
#ifdef F_SETFD
        (void) fcntl (fileno (ofile), F_SETFD, 0);
#endif
        break;

      case 'p':
        pid = strtoul (optarg, NULL, 10);
        break;

      case 's':
        samplerate = strtoul (optarg, NULL, 10);
        if (samplerate < 1 || samplerate > 1000)
          error (0, "sample rate must be between 1 and 1000 inclusive");
        break;

      case 'v':
        verbose ^= 1;
        verbose |= debugging; // debugging turns on verbose too
        break;

      case 'V':
        /* Print version info and exit. */
        print_version ();

      case 'w':
        new_window ^= 1;
        break;

      default:
        note ("Try `%s --help' for more information.\n", pgm);
        exit (1);
      }

  if (pid && argv[optind])
    error (0, "cannot provide both a command line and a process id");

  if (!pid && !argv[optind])
    error (0, "must provide either a command line or a process id");

  /* Honor user-supplied gmon file name prefix, if available. */
  ptr = getenv ("GMON_OUT_PREFIX");
  if (ptr && strlen (ptr) > 0)
    prefix = ptr;

  drive_map = (void *) cygwin_internal (CW_ALLOC_DRIVE_MAP);
  ret = doprofile (ofile, pid, argv + optind);
  if (drive_map)
    cygwin_internal (CW_FREE_DRIVE_MAP, drive_map);

  if (ofile && ofile != stdout)
    fclose (ofile);
  return (ret);
}
