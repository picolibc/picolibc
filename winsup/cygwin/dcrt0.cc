/* dcrt0.cc -- essentially the main() for the Cygwin dll

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005,
   2006, 2007, 2008 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "miscfuncs.h"
#include <unistd.h>
#include <stdlib.h>
#include "glob.h"
#include <ctype.h>
#include "sigproc.h"
#include "pinfo.h"
#include "cygerrno.h"
#define NEED_VFORK
#include "perprocess.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "child_info_magic.h"
#include "cygtls.h"
#include "shared_info.h"
#include "cygwin_version.h"
#include "dll_init.h"
#include "heap.h"
#include "tls_pbuf.h"

#define MAX_AT_FILE_LEVEL 10

#define PREMAIN_LEN (sizeof (user_data->premain) / sizeof (user_data->premain[0]))

extern "C" void cygwin_exit (int) __attribute__ ((noreturn));

HANDLE NO_COPY hMainProc = (HANDLE) -1;
HANDLE NO_COPY hMainThread;
HANDLE NO_COPY hProcToken;
HANDLE NO_COPY hProcImpToken;

muto NO_COPY lock_process::locker;

bool display_title;
bool strip_title_path;
bool allow_glob = true;
bool NO_COPY in_forkee;

int __argc_safe;
int _declspec(dllexport) __argc;
char _declspec(dllexport) **__argv;
#ifdef NEWVFORK
vfork_save NO_COPY *main_vfork;
#endif

static int NO_COPY envc;
char NO_COPY **envp;

extern "C" void __sinit (_reent *);

_cygtls NO_COPY *_main_tls;

bool NO_COPY cygwin_finished_initializing;

MTinterface _mtinterf;

bool NO_COPY _cygwin_testing;

char NO_COPY almost_null[1];

extern "C"
{
  /* This is an exported copy of environ which can be used by DLLs
     which use cygwin.dll.  */
  char **__cygwin_environ;
  char ***main_environ = &__cygwin_environ;
  /* __progname used in getopt error message */
  char *__progname;
  struct per_process __cygwin_user_data =
  {/* initial_sp */ 0, /* magic_biscuit */ 0,
   /* dll_major */ CYGWIN_VERSION_DLL_MAJOR,
   /* dll_major */ CYGWIN_VERSION_DLL_MINOR,
   /* impure_ptr_ptr */ NULL, /* envptr */ NULL,
   /* malloc */ malloc, /* free */ free,
   /* realloc */ realloc,
   /* fmode_ptr */ NULL, /* main */ NULL, /* ctors */ NULL,
   /* dtors */ NULL, /* data_start */ NULL, /* data_end */ NULL,
   /* bss_start */ NULL, /* bss_end */ NULL,
   /* calloc */ calloc,
   /* premain */ {NULL, NULL, NULL, NULL},
   /* run_ctors_p */ 0,
   /* unused */ {0, 0, 0, 0, 0, 0, 0},
   /* UNUSED forkee */ 0,
   /* hmodule */ NULL,
   /* api_major */ CYGWIN_VERSION_API_MAJOR,
   /* api_minor */ CYGWIN_VERSION_API_MINOR,
   /* unused2 */ {0, 0, 0, 0, 0, 0},
   /* threadinterface */ &_mtinterf,
   /* impure_ptr */ _GLOBAL_REENT,
  };
  bool ignore_case_with_glob;
  int __declspec (dllexport) _check_for_executable = true;
#ifdef DEBUGGING
  int pinger;
#endif
};

int NO_COPY __api_fatal_exit_val = 1;
char *old_title;
char title_buf[TITLESIZE + 1];

static void
do_global_dtors ()
{
  void (**pfunc) () = user_data->dtors;
  if (pfunc)
    {
      user_data->dtors = NULL;
      while (*++pfunc)
	(*pfunc) ();
    }
}

static void __stdcall
do_global_ctors (void (**in_pfunc)(), int force)
{
  if (!force && in_forkee)
    return;		// inherit constructed stuff from parent pid

  /* Run ctors backwards, so skip the first entry and find how many
     there are, then run them. */

  void (**pfunc) () = in_pfunc;

  while (*++pfunc)
    ;
  while (--pfunc > in_pfunc)
    (*pfunc) ();
}

/*
 * Replaces @file in the command line with the contents of the file.
 * There may be multiple @file's in a single command line
 * A \@file is replaced with @file so that echo \@foo would print
 * @foo and not the contents of foo.
 */
static bool __stdcall
insert_file (char *name, char *&cmd)
{
  HANDLE f;
  DWORD size;
  tmp_pathbuf tp;

  PWCHAR wname = tp.w_get ();
  sys_mbstowcs (wname, NT_MAX_PATH, name + 1);
  f = CreateFileW (wname,
		   GENERIC_READ,	 /* open for reading	*/
		   FILE_SHARE_READ,      /* share for reading	*/
		   &sec_none_nih,	 /* default security	*/
		   OPEN_EXISTING,	 /* existing file only	*/
		   FILE_ATTRIBUTE_NORMAL,/* normal file		*/
		   NULL);		 /* no attr. template	*/

  if (f == INVALID_HANDLE_VALUE)
    {
      debug_printf ("couldn't open file '%s', %E", name);
      return false;
    }

  /* This only supports files up to about 4 billion bytes in
     size.  I am making the bold assumption that this is big
     enough for this feature */
  size = GetFileSize (f, NULL);
  if (size == 0xFFFFFFFF)
    {
      debug_printf ("couldn't get file size for '%s', %E", name);
      return false;
    }

  int new_size = strlen (cmd) + size + 2;
  char *tmp = (char *) malloc (new_size);
  if (!tmp)
    {
      debug_printf ("malloc failed, %E");
      return false;
    }

  /* realloc passed as it should */
  DWORD rf_read;
  BOOL rf_result;
  rf_result = ReadFile (f, tmp, size, &rf_read, NULL);
  CloseHandle (f);
  if (!rf_result || (rf_read != size))
    {
      debug_printf ("ReadFile failed, %E");
      return false;
    }

  tmp[size++] = ' ';
  strcpy (tmp + size, cmd);
  cmd = tmp;
  return true;
}

static inline int
isquote (char c)
{
  char ch = c;
  return ch == '"' || ch == '\'';
}

/* Step over a run of characters delimited by quotes */
static /*__inline*/ char *
quoted (char *cmd, int winshell)
{
  char *p;
  char quote = *cmd;

  if (!winshell)
    {
      char *p;
      strcpy (cmd, cmd + 1);
      if (*(p = strechr (cmd, quote)))
	strcpy (p, p + 1);
      return p;
    }

  const char *s = quote == '\'' ? "'" : "\\\"";
  /* This must have been run from a Windows shell, so preserve
     quotes for globify to play with later. */
  while (*cmd && *++cmd)
    if ((p = strpbrk (cmd, s)) == NULL)
      {
	cmd = strchr (cmd, '\0');	// no closing quote
	break;
      }
    else if (*p == '\\')
      cmd = ++p;
    else if (quote == '"' && p[1] == '"')
      {
	*p = '\\';
	cmd = ++p;			// a quoted quote
      }
    else
      {
	cmd = p + 1;		// point to after end
	break;
      }
  return cmd;
}

/* Perform a glob on word if it contains wildcard characters.
   Also quote every character between quotes to force glob to
   treat the characters literally. */
static int __stdcall
globify (char *word, char **&argv, int &argc, int &argvlen)
{
  if (*word != '~' && strpbrk (word, "?*[\"\'(){}") == NULL)
    return 0;

  int n = 0;
  char *p, *s;
  int dos_spec = isdrive (word);
  if (!dos_spec && isquote (*word) && word[1] && word[2])
    dos_spec = isdrive (word + 1);

  /* We'll need more space if there are quoting characters in
     word.  If that is the case, doubling the size of the
     string should provide more than enough space. */
  if (strpbrk (word, "'\""))
    n = strlen (word);
  char pattern[strlen (word) + ((dos_spec + 1) * n) + 1];

  /* Fill pattern with characters from word, quoting any
     characters found within quotes. */
  for (p = pattern, s = word; *s != '\000'; s++, p++)
    if (!isquote (*s))
      {
	if (dos_spec && *s == '\\')
	  *p++ = '\\';
	*p = *s;
      }
    else
      {
	char quote = *s;
	while (*++s && *s != quote)
	  {
	    if (dos_spec || *s != '\\')
	      /* nothing */;
	    else if (s[1] == quote || s[1] == '\\')
	      s++;
	    *p++ = '\\';
	    *p++ = *s;
	  }
	if (*s == quote)
	  p--;
	if (*s == '\0')
	    break;
      }

  *p = '\0';

  glob_t gl;
  gl.gl_offs = 0;

  /* Attempt to match the argument.  Return just word (minus quoting) if no match. */
  if (glob (pattern, GLOB_TILDE | GLOB_NOCHECK | GLOB_BRACE | GLOB_QUOTE, NULL, &gl) || !gl.gl_pathc)
    return 0;

  /* Allocate enough space in argv for the matched filenames. */
  n = argc;
  if ((argc += gl.gl_pathc) > argvlen)
    {
      argvlen = argc + 10;
      argv = (char **) realloc (argv, (1 + argvlen) * sizeof (argv[0]));
    }

  /* Copy the matched filenames to argv. */
  char **gv = gl.gl_pathv;
  char **av = argv + n;
  while (*gv)
    {
      debug_printf ("argv[%d] = '%s'", n++, *gv);
      *av++ = *gv++;
    }

  /* Clean up after glob. */
  free (gl.gl_pathv);
  return 1;
}

/* Build argv, argc from string passed from Windows.  */

static void __stdcall
build_argv (char *cmd, char **&argv, int &argc, int winshell)
{
  int argvlen = 0;
  int nesting = 0;		// monitor "nesting" from insert_file

  argc = 0;
  argvlen = 0;
  argv = NULL;

  /* Scan command line until there is nothing left. */
  while (*cmd)
    {
      /* Ignore spaces */
      if (issep (*cmd))
	{
	  cmd++;
	  continue;
	}

      /* Found the beginning of an argument. */
      char *word = cmd;
      char *sawquote = NULL;
      while (*cmd)
	{
	  if (*cmd != '"' && (!winshell || *cmd != '\''))
	    cmd++;		// Skip over this character
	  else
	    /* Skip over characters until the closing quote */
	    {
	      sawquote = cmd;
	      cmd = quoted (cmd, winshell && argc > 0);
	    }
	  if (issep (*cmd))	// End of argument if space
	    break;
	}
      if (*cmd)
	*cmd++ = '\0';		// Terminate `word'

      /* Possibly look for @file construction assuming that this isn't
	 the very first argument and the @ wasn't quoted */
      if (argc && sawquote != word && *word == '@')
	{
	  if (++nesting > MAX_AT_FILE_LEVEL)
	    api_fatal ("Too many levels of nesting for %s", word);
	  if (insert_file (word, cmd))
	      continue;			// There's new stuff in cmd now
	}

      /* See if we need to allocate more space for argv */
      if (argc >= argvlen)
	{
	  argvlen = argc + 10;
	  argv = (char **) realloc (argv, (1 + argvlen) * sizeof (argv[0]));
	}

      /* Add word to argv file after (optional) wildcard expansion. */
      if (!winshell || !argc || !globify (word, argv, argc, argvlen))
	{
	  debug_printf ("argv[%d] = '%s'", argc, word);
	  argv[argc++] = word;
	}
    }

  argv[argc] = NULL;

  debug_printf ("argc %d", argc);
}

/* sanity and sync check */
void __stdcall
check_sanity_and_sync (per_process *p)
{
  /* Sanity check to make sure developers didn't change the per_process    */
  /* struct without updating SIZEOF_PER_PROCESS [it makes them think twice */
  /* about changing it].						   */
  if (sizeof (per_process) != SIZEOF_PER_PROCESS)
    api_fatal ("per_process sanity check failed");

  /* Make sure that the app and the dll are in sync. */

  /* Complain if older than last incompatible change */
  if (p->dll_major < CYGWIN_VERSION_DLL_EPOCH)
    api_fatal ("cygwin DLL and APP are out of sync -- DLL version mismatch %d < %d",
	       p->dll_major, CYGWIN_VERSION_DLL_EPOCH);

  /* magic_biscuit != 0 if using the old style version numbering scheme.  */
  if (p->magic_biscuit != SIZEOF_PER_PROCESS)
    api_fatal ("Incompatible cygwin .dll -- incompatible per_process info %d != %d",
	       p->magic_biscuit, SIZEOF_PER_PROCESS);

  /* Complain if incompatible API changes made */
  if (p->api_major > cygwin_version.api_major)
    api_fatal ("cygwin DLL and APP are out of sync -- API version mismatch %d > %d",
	       p->api_major, cygwin_version.api_major);
}

child_info NO_COPY *child_proc_info = NULL;

#define CYGWIN_GUARD (PAGE_EXECUTE_READWRITE | PAGE_GUARD)

void
child_info_fork::alloc_stack_hard_way (volatile char *b)
{
  void *new_stack_pointer;
  MEMORY_BASIC_INFORMATION m;
  void *newbase;
  int newlen;
  bool guard;

  if (!VirtualQuery ((LPCVOID) &b, &m, sizeof m))
    api_fatal ("fork: couldn't get stack info, %E");

  LPBYTE curbot = (LPBYTE) m.BaseAddress + m.RegionSize;

  if (stacktop > (LPBYTE) m.AllocationBase && stacktop < curbot)
    {
      newbase = curbot;
      newlen = (LPBYTE) stackbottom - (LPBYTE) curbot;
      guard = false;
    }
  else
    {
      newbase = (LPBYTE) stacktop - (128 * 1024);
      newlen = (LPBYTE) stackbottom - (LPBYTE) newbase;
      guard = true;
    }

  if (!VirtualAlloc (newbase, newlen, MEM_RESERVE, PAGE_NOACCESS))
    api_fatal ("fork: can't reserve memory for stack %p - %p, %E",
		stacktop, stackbottom);
  new_stack_pointer = (void *) ((LPBYTE) stackbottom - (stacksize += 8192));
  if (!VirtualAlloc (new_stack_pointer, stacksize, MEM_COMMIT,
		     PAGE_EXECUTE_READWRITE))
    api_fatal ("fork: can't commit memory for stack %p(%d), %E",
	       new_stack_pointer, stacksize);
  if (!VirtualQuery ((LPCVOID) new_stack_pointer, &m, sizeof m))
    api_fatal ("fork: couldn't get new stack info, %E");

  if (guard)
    {
      m.BaseAddress = (LPBYTE) m.BaseAddress - 1;
      if (!VirtualAlloc ((LPVOID) m.BaseAddress, 1, MEM_COMMIT,
			 CYGWIN_GUARD))
	api_fatal ("fork: couldn't allocate new stack guard page %p, %E",
		   m.BaseAddress);
    }
  if (!VirtualQuery ((LPCVOID) m.BaseAddress, &m, sizeof m))
    api_fatal ("fork: couldn't get new stack info, %E");
  stacktop = m.BaseAddress;
  b[0] = '\0';
}

void *getstack (void *) __attribute__ ((noinline));
volatile char *
getstack (volatile char * volatile p)
{
  *p ^= 1;
  *p ^= 1;
  return p - 4096;
}

/* extend the stack prior to fork longjmp */

void
child_info_fork::alloc_stack ()
{
  volatile char * volatile esp;
  __asm__ volatile ("movl %%esp,%0": "=r" (esp));
  if (_tlsbase != stackbottom)
    alloc_stack_hard_way (esp);
  else
    {
      char *st = (char *) stacktop - 4096;
      while (_tlstop >= st)
	esp = getstack (esp);
      stacksize = 0;
    }
}

extern "C" void
break_here ()
{
  static int NO_COPY sent_break;
  if (!sent_break++)
    DebugBreak ();
  debug_printf ("break here");
}

static void
initial_env ()
{
  if (GetEnvironmentVariableA ("CYGWIN_TESTING", NULL, 0))
    _cygwin_testing = 1;

#ifdef DEBUGGING
  char buf[NT_MAX_PATH];
  DWORD len;

  if (GetEnvironmentVariableA ("CYGWIN_SLEEP", buf, sizeof (buf) - 1))
    {
      DWORD ms = atoi (buf);
      console_printf ("Sleeping %d, pid %u %P\n", ms, GetCurrentProcessId ());
      Sleep (ms);
      if (!strace.active () && !dynamically_loaded)
	strace.hello ();
    }
  if (GetEnvironmentVariableA ("CYGWIN_DEBUG", buf, sizeof (buf) - 1))
    {
      char buf1[NT_MAX_PATH];
      len = GetModuleFileName (NULL, buf1, NT_MAX_PATH);
      strlwr (buf1);
      strlwr (buf);
      char *p = strpbrk (buf, ":=");
      if (!p)
	p = (char *) "gdb.exe -nw";
      else
	*p++ = '\0';
      if (strstr (buf1, buf))
	{
	  error_start_init (p);
	  try_to_debug ();
	  console_printf ("*** Sending Break.  gdb may issue spurious SIGTRAP message.\n");
	  break_here ();
	}
    }
#endif

}

child_info *
get_cygwin_startup_info ()
{
  STARTUPINFO si;

  GetStartupInfo (&si);
  child_info *res = (child_info *) si.lpReserved2;

  if (si.cbReserved2 < EXEC_MAGIC_SIZE || !res
      || res->intro != PROC_MAGIC_GENERIC || res->magic != CHILD_INFO_MAGIC)
    res = NULL;
  else
    {
      if ((res->intro & OPROC_MAGIC_MASK) == OPROC_MAGIC_GENERIC)
	multiple_cygwin_problem ("proc intro", res->intro, 0);
      else if (res->cygheap != (void *) &_cygheap_start)
	multiple_cygwin_problem ("cygheap base", (DWORD) res->cygheap,
				 (DWORD) &_cygheap_start);

      unsigned should_be_cb = 0;
      switch (res->type)
	{
	  case _PROC_FORK:
	    in_forkee = true;
	    should_be_cb = sizeof (child_info_fork);
	    /* fall through */;
	  case _PROC_SPAWN:
	  case _PROC_EXEC:
	    if (!should_be_cb)
	      should_be_cb = sizeof (child_info_spawn);
	    if (should_be_cb != res->cb)
	      multiple_cygwin_problem ("proc size", res->cb, should_be_cb);
	    else if (sizeof (fhandler_union) != res->fhandler_union_cb)
	      multiple_cygwin_problem ("fhandler size", res->fhandler_union_cb, sizeof (fhandler_union));
	    if (res->isstraced ())
	      {
		res->ready (false);
		for (unsigned i = 0; !being_debugged () && i < 10000; i++)
		  low_priority_sleep (0);
		strace.hello ();
	      }
	    break;
	  default:
	    system_printf ("unknown exec type %d", res->type);
	    /* intentionally fall through */
	  case _PROC_WHOOPS:
	    res = NULL;
	    break;
	}
    }

  return res;
}

#define dll_data_start &_data_start__
#define dll_data_end &_data_end__
#define dll_bss_start &_bss_start__
#define dll_bss_end &_bss_end__

void
child_info_fork::handle_fork ()
{
  cygheap_fixup_in_child (false);
  memory_init ();
  set_myself (NULL);
  myself->uid = cygheap->user.real_uid;
  myself->gid = cygheap->user.real_gid;

  child_copy (parent, false,
	      "dll data", dll_data_start, dll_data_end,
	      "dll bss", dll_bss_start, dll_bss_end,
	      "user heap", cygheap->user_heap.base, cygheap->user_heap.ptr,
	      NULL);
  /* step 2 now that the dll has its heap filled in, we can fill in the
     user's data and bss since user_data is now filled out. */
  child_copy (parent, false,
	      "data", user_data->data_start, user_data->data_end,
	      "bss", user_data->bss_start, user_data->bss_end,
	      NULL);

  if (fixup_mmaps_after_fork (parent))
    api_fatal ("recreate_mmaps_after_fork_failed");
}

void
child_info_spawn::handle_spawn ()
{
  extern void fixup_lockf_after_exec ();
  HANDLE h;
  cygheap_fixup_in_child (true);
  memory_init ();
  if (!moreinfo->myself_pinfo ||
      !DuplicateHandle (hMainProc, moreinfo->myself_pinfo, hMainProc, &h, 0,
			FALSE, DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE))
    h = NULL;
  set_myself (h);
  __argc = moreinfo->argc;
  __argv = moreinfo->argv;
  envp = moreinfo->envp;
  envc = moreinfo->envc;
  if (!dynamically_loaded)
    cygheap->fdtab.fixup_after_exec ();
  if (__stdin >= 0)
    cygheap->fdtab.move_fd (__stdin, 0);
  if (__stdout >= 0)
    cygheap->fdtab.move_fd (__stdout, 1);
  cygheap->user.groups.clear_supp ();

  ready (true);

  /* Need to do this after debug_fixup_after_fork_exec or DEBUGGING handling of
     handles might get confused. */
  CloseHandle (child_proc_info->parent);
  child_proc_info->parent = NULL;

  signal_fixup_after_exec ();
  if (moreinfo->old_title)
    {
      old_title = strcpy (title_buf, moreinfo->old_title);
      cfree (moreinfo->old_title);
    }
  fixup_lockf_after_exec ();
}

void __stdcall
dll_crt0_0 ()
{
  init_global_security ();
  initial_env ();

  SetErrorMode (SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);

  /* Initialize signal processing here, early, in the hopes that the creation
     of a thread early in the process will cause more predictability in memory
     layout for the main thread. */
  sigproc_init ();

  lock_process::init ();
  _impure_ptr = _GLOBAL_REENT;
  _impure_ptr->_stdin = &_impure_ptr->__sf[0];
  _impure_ptr->_stdout = &_impure_ptr->__sf[1];
  _impure_ptr->_stderr = &_impure_ptr->__sf[2];
  _impure_ptr->_current_locale = "C";
  user_data->impure_ptr = _impure_ptr;
  user_data->impure_ptr_ptr = &_impure_ptr;

  if (!DuplicateHandle (GetCurrentProcess (), GetCurrentProcess (),
		       GetCurrentProcess (), &hMainProc, 0, FALSE,
			DUPLICATE_SAME_ACCESS))
    hMainProc = GetCurrentProcess ();

  DuplicateHandle (hMainProc, GetCurrentThread (), hMainProc,
		   &hMainThread, 0, false, DUPLICATE_SAME_ACCESS);

  OpenProcessToken (hMainProc, MAXIMUM_ALLOWED, &hProcToken);
  set_cygwin_privileges (hProcToken);

  device::init ();
  do_global_ctors (&__CTOR_LIST__, 1);
  cygthread::init ();

  child_proc_info = get_cygwin_startup_info ();
  if (!child_proc_info)
    memory_init ();
  else
    {
      cygwin_user_h = child_proc_info->user_h;
      switch (child_proc_info->type)
	{
	  case _PROC_FORK:
	    fork_info->handle_fork ();
	    break;
	  case _PROC_SPAWN:
	  case _PROC_EXEC:
	    spawn_info->handle_spawn ();
	    break;
	}
    }

  user_data->threadinterface->Init ();

  _cygtls::init ();

  /* Initialize events */
  events_init ();
  tty_list::init_session ();

  debug_printf ("finished dll_crt0_0 initialization");
}

/* Take over from libc's crt0.o and start the application. Note the
   various special cases when Cygwin DLL is being runtime loaded (as
   opposed to being link-time loaded by Cygwin apps) from a non
   cygwin app via LoadLibrary.  */
void
dll_crt0_1 (void *)
{
  check_sanity_and_sync (user_data);
  malloc_init ();
#ifdef CGF
  int i = 0;
  const int n = 2 * 1024 * 1024;
  while (i--)
    small_printf ("cmalloc returns %p\n", cmalloc (HEAP_STR, n));
#endif

  ProtectHandle (hMainProc);
  ProtectHandle (hMainThread);

  cygheap->cwd.init ();

  /* Initialize pthread mainthread when not forked and it is safe to call new,
     otherwise it is reinitalized in fixup_after_fork */
  if (!in_forkee)
    pthread::init_mainthread ();

#ifdef DEBUGGING
  strace.microseconds ();
#endif

  create_signal_arrived (); /* FIXME: move into wait_sig? */

  /* Initialize debug muto, if DLL is built with --enable-debugging.
     Need to do this before any helper threads start. */
  debug_init ();

#ifdef NEWVFORK
  cygheap->fdtab.vfork_child_fixup ();
  main_vfork = vfork_storage.create ();
#endif

  cygbench ("pre-forkee");
  if (in_forkee)
    {
      /* If we've played with the stack, stacksize != 0.  That means that
	 fork() was invoked from other than the main thread.  Make sure that
	 frame pointer is referencing the new stack so that the OS knows what
	 to do when it needs to increase the size of the stack.

	 NOTE: Don't do anything that involves the stack until you've completed
	 this step. */
      if (fork_info->stacksize)
	{
	  _tlsbase = (char *) fork_info->stackbottom;
	  _tlstop = (char *) fork_info->stacktop;
	  _my_tls.init_exception_handler (_cygtls::handle_exceptions);
	}

      longjmp (fork_info->jmp, true);
    }

#ifdef DEBUGGING
  {
  extern void fork_init ();
  fork_init ();
  }
#endif
  pinfo_init (envp, envc);

  if (!old_title && GetConsoleTitle (title_buf, TITLESIZE))
    old_title = title_buf;

  /* Allocate cygheap->fdtab */
  dtable_init ();

  uinfo_init ();	/* initialize user info */

  wait_for_sigthread ();
  extern DWORD threadfunc_ix;
  if (!threadfunc_ix)
    system_printf ("internal error: couldn't determine location of thread function on stack.  Expect signal problems.");

  /* Connect to tty. */
  tty::init_session ();

  if (!__argc)
    {
      PWCHAR wline = GetCommandLineW ();
      size_t size = sys_wcstombs (NULL, 0, wline);
      char *line = (char *) alloca (size);
      sys_wcstombs (line, size, wline);

      /* Scan the command line and build argv.  Expand wildcards if not
	 called from another cygwin process. */
      build_argv (line, __argv, __argc,
		  NOTSTATE (myself, PID_CYGPARENT) && allow_glob);

      /* Convert argv[0] to posix rules if it's currently blatantly
	 win32 style. */
      if ((strchr (__argv[0], ':')) || (strchr (__argv[0], '\\')))
	{
	  char *new_argv0 = (char *) malloc (NT_MAX_PATH);
	  cygwin_conv_path (CCP_WIN_A_TO_POSIX | CCP_RELATIVE, __argv[0],
			    new_argv0, NT_MAX_PATH);
	  __argv[0] = (char *) realloc (new_argv0, strlen (new_argv0) + 1);
	}
    }

  __argc_safe = __argc;
  if (user_data->premain[0])
    for (unsigned int i = 0; i < PREMAIN_LEN / 2; i++)
      user_data->premain[i] (__argc, __argv, user_data);

  /* Set up standard fds in file descriptor table. */
  cygheap->fdtab.stdio_init ();

  /* Set up __progname for getopt error call. */
  if (__argv[0] && (__progname = strrchr (__argv[0], '/')))
    ++__progname;
  else
    __progname = __argv[0];
  if (__progname)
    {
      char *cp = strchr (__progname, '\0') - 4;
      if (cp > __progname && ascii_strcasematch (cp, ".exe"))
	*cp = '\0';
    }

  /* Set new console title if appropriate. */

  if (display_title && !dynamically_loaded)
    {
      char *cp = __progname;
      if (strip_title_path)
	for (char *ptr = cp; *ptr && *ptr != ' '; ptr++)
	  if (isdirsep (*ptr))
	    cp = ptr + 1;
      set_console_title (cp);
    }

  cygwin_finished_initializing = true;
  /* Call init of loaded dlls. */
  dlls.init ();

  /* Execute any specified "premain" functions */
  if (user_data->premain[PREMAIN_LEN / 2])
    for (unsigned int i = PREMAIN_LEN / 2; i < PREMAIN_LEN; i++)
      user_data->premain[i] (__argc, __argv, user_data);

  debug_printf ("user_data->main %p", user_data->main);

  if (dynamically_loaded)
    {
      set_errno (0);
      return;
    }

  /* Disable case-insensitive globbing */
  ignore_case_with_glob = false;

  set_errno (0);

  MALLOC_CHECK;
  cygbench (__progname);

  /* Flush signals and ensure that signal thread is up and running. Can't
     do this for noncygwin case since the signal thread is blocked due to
     LoadLibrary serialization. */
  ld_preload ();
  if (user_data->main)
    cygwin_exit (user_data->main (__argc, __argv, *user_data->envptr));
}

extern "C" void __stdcall
_dll_crt0 ()
{
  main_environ = user_data->envptr;
  if (in_forkee)
    fork_info->alloc_stack ();
  else
    __sinit (_impure_ptr);

  _main_tls = &_my_tls;
  _main_tls->call ((DWORD (*) (void *, void *)) dll_crt0_1, NULL);
}

void
dll_crt0 (per_process *uptr)
{
  /* Set the local copy of the pointer into the user space. */
  if (!in_forkee && uptr && uptr != user_data)
    {
      memcpy (user_data, uptr, per_process_overwrite);
      *(user_data->impure_ptr_ptr) = _GLOBAL_REENT;
    }
  _dll_crt0 ();
}

/* This must be called by anyone who uses LoadLibrary to load cygwin1.dll.
   You must have CYGTLS_PADSIZE bytes reserved at the bottom of the stack
   calling this function, and that storage must not be overwritten until you
   unload cygwin1.dll, as it is used for _my_tls.  It is best to load
   cygwin1.dll before spawning any additional threads in your process.

   See winsup/testsuite/cygload for an example of how to use cygwin1.dll
   from MSVC and non-cygwin MinGW applications.  */
extern "C" void
cygwin_dll_init ()
{
  static char **envp;
  static int _fmode;

  user_data->magic_biscuit = sizeof (per_process);

  user_data->envptr = &envp;
  user_data->fmode_ptr = &_fmode;

  _dll_crt0 ();
}

extern "C" void
__main (void)
{
  do_global_ctors (user_data->ctors, false);
  atexit (do_global_dtors);
}

exit_states NO_COPY exit_state;

void __stdcall
do_exit (int status)
{
  syscall_printf ("do_exit (%d), exit_state %d", status, exit_state);

#ifdef NEWVFORK
  vfork_save *vf = vfork_storage.val ();
  if (vf != NULL && vf->pid < 0)
    {
      exit_state = ES_NOT_EXITING;
      vf->restore_exit (status);
    }
#endif

  lock_process until_exit (true);

  if (exit_state < ES_GLOBAL_DTORS)
    {
      exit_state = ES_GLOBAL_DTORS;
      dll_global_dtors ();
    }

  if (exit_state < ES_EVENTS_TERMINATE)
    {
      exit_state = ES_EVENTS_TERMINATE;
      events_terminate ();
    }

  UINT n = (UINT) status;
  if (exit_state < ES_THREADTERM)
    {
      exit_state = ES_THREADTERM;
      cygthread::terminate ();
    }

  if (exit_state < ES_SIGNAL)
    {
      exit_state = ES_SIGNAL;
      signal (SIGCHLD, SIG_IGN);
      signal (SIGHUP, SIG_IGN);
      signal (SIGINT, SIG_IGN);
      signal (SIGQUIT, SIG_IGN);
    }

  if (exit_state < ES_CLOSEALL)
    {
      exit_state = ES_CLOSEALL;
      close_all_files ();
    }

  myself->stopsig = 0;

  if (exit_state < ES_HUP_PGRP)
    {
      exit_state = ES_HUP_PGRP;
      /* Kill orphaned children on group leader exit */
      if (myself->has_pgid_children && myself->pid == myself->pgid)
	{
	  siginfo_t si = {0};
	  si.si_signo = -SIGHUP;
	  si.si_code = SI_KERNEL;
	  sigproc_printf ("%d == pgrp %d, send SIG{HUP,CONT} to stopped children",
			  myself->pid, myself->pgid);
	  kill_pgrp (myself->pgid, si);
	}
    }

  if (exit_state < ES_HUP_SID)
    {
      exit_state = ES_HUP_SID;
      /* Kill the foreground process group on session leader exit */
      if (getpgrp () > 0 && myself->pid == myself->sid && real_tty_attached (myself))
	{
	  tty *tp = cygwin_shared->tty[myself->ctty];
	  sigproc_printf ("%d == sid %d, send SIGHUP to children",
			  myself->pid, myself->sid);

	/* CGF FIXME: This can't be right. */
	  if (tp->getsid () == myself->sid)
	    tp->kill_pgrp (SIGHUP);
	}

    }

  if (exit_state < ES_TITLE)
    {
      exit_state = ES_TITLE;
      /* restore console title */
      if (old_title && display_title)
	set_console_title (old_title);
    }

  if (exit_state < ES_TTY_TERMINATE)
    {
      exit_state = ES_TTY_TERMINATE;
      cygwin_shared->tty.terminate ();
    }

  myself.exit (n);
}

static NO_COPY muto atexit_lock;

extern "C" int
cygwin_atexit (void (*function)(void))
{
  int res;
  atexit_lock.init ("atexit_lock");
  atexit_lock.acquire ();
  res = atexit (function);
  atexit_lock.release ();
  return res;
}

extern "C" void
cygwin_exit (int n)
{
  dll_global_dtors ();
  if (atexit_lock)
    atexit_lock.acquire ();
  exit (n);
}

extern "C" void
_exit (int n)
{
  do_exit (((DWORD) n & 0xff) << 8);
}

extern "C" void
__api_fatal (const char *fmt, ...)
{
  char buf[4096];
  va_list ap;

  va_start (ap, fmt);
  int n = __small_sprintf (buf, "%P: *** fatal error - ");
  __small_vsprintf (buf + n, fmt, ap);
  va_end (ap);
  strace.prntf (_STRACE_SYSTEM, NULL, "%s", buf);

#ifdef DEBUGGING
  try_to_debug ();
#endif
  myself.exit (__api_fatal_exit_val);
}

void
multiple_cygwin_problem (const char *what, unsigned magic_version, unsigned version)
{
  if (_cygwin_testing && (strstr (what, "proc") || strstr (what, "cygheap")))
    {
      child_proc_info->type = _PROC_WHOOPS;
      return;
    }

  if (GetEnvironmentVariableA ("CYGWIN_MISMATCH_OK", NULL, 0))
    return;

  if (CYGWIN_VERSION_MAGIC_VERSION (magic_version) == version)
    system_printf ("%s magic number mismatch detected - %p/%p", what, magic_version, version);
  else
    api_fatal ("%s mismatch detected - %p/%p.\n\
This problem is probably due to using incompatible versions of the cygwin DLL.\n\
Search for cygwin1.dll using the Windows Start->Find/Search facility\n\
and delete all but the most recent version.  The most recent version *should*\n\
reside in x:\\cygwin\\bin, where 'x' is the drive on which you have\n\
installed the cygwin distribution.  Rebooting is also suggested if you\n\
are unable to find another cygwin DLL.",
	       what, magic_version, version);
}

#ifdef DEBUGGING
void __stdcall
cygbench (const char *s)
{
  if (GetEnvironmentVariableA ("CYGWIN_BENCH", NULL, 0))
    small_printf ("%05d ***** %s : %10d\n", GetCurrentProcessId (), s, strace.microseconds ());
}
#endif
