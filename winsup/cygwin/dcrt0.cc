/* dcrt0.cc -- essentially the main() for the Cygwin dll

   Copyright 1996, 1997, 1998, 1999, 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <unistd.h>
#include <stdlib.h>
#include "glob.h"
#include "exceptions.h"
#include "dll_init.h"
#include "autoload.h"
#include <ctype.h>
#include "sync.h"
#include "sigproc.h"
#include "perthread.h"
#include "pinfo.h"
#include "cygheap.h"
#include "heap.h"
#include "cygerrno.h"
#include "fhandler.h"
#include "child_info.h"
#include "path.h"
#include "dtable.h"
#include "thread.h"
#include "tty.h"
#include "shared_info.h"

#define MAX_AT_FILE_LEVEL 10

#define PREMAIN_LEN (sizeof(user_data->premain) / sizeof (user_data->premain[0]))

HANDLE NO_COPY hMainProc = NULL;
HANDLE NO_COPY hMainThread = NULL;

sigthread NO_COPY mainthread;		// ID of the main thread

per_thread_waitq NO_COPY waitq_storage;
per_thread_vfork NO_COPY vfork_storage;
per_thread_signal_dispatch NO_COPY signal_dispatch_storage;

per_thread NO_COPY *threadstuff[] = {&waitq_storage,
				     &vfork_storage,
				     &signal_dispatch_storage,
				     NULL};

BOOL display_title = FALSE;
BOOL strip_title_path = FALSE;
BOOL allow_glob = TRUE;

HANDLE NO_COPY parent_alive = NULL;
int cygwin_finished_initializing = 0;

/* Used in SIGTOMASK for generating a bit for insertion into a sigset_t.
   This is subtracted from the signal number prior to shifting the bit.
   In older versions of cygwin, the signal was used as-is to shift the
   bit for masking.  So, we'll temporarily detect this and set it to zero
   for programs that are linked using older cygwins.  This is just a stopgap
   measure to allow an orderly transfer to the new, correct sigmask method. */
unsigned int signal_shift_subtract = 1;

ResourceLocks _reslock NO_COPY;
MTinterface _mtinterf NO_COPY;

extern "C"
{
  void *export_malloc (unsigned int);
  void export_free (void *);
  void *export_realloc (void *, unsigned int);
  void *export_calloc (unsigned int, unsigned int);

  /* This is an exported copy of environ which can be used by DLLs
     which use cygwin.dll.  */
  char **__cygwin_environ;
  char ***main_environ;
  /* __progname used in getopt error message */
  char *__progname = NULL;
  struct _reent reent_data;
  struct per_process __cygwin_user_data =
  {/* initial_sp */ 0, /* magic_biscuit */ 0,
   /* dll_major */ CYGWIN_VERSION_DLL_MAJOR,
   /* dll_major */ CYGWIN_VERSION_DLL_MINOR,
   /* impure_ptr_ptr */ NULL, /* envptr */ NULL,
   /* malloc */ export_malloc, /* free */ export_free,
   /* realloc */ export_realloc,
   /* fmode_ptr */ NULL, /* main */ NULL, /* ctors */ NULL,
   /* dtors */ NULL, /* data_start */ NULL, /* data_end */ NULL,
   /* bss_start */ NULL, /* bss_end */ NULL,
   /* calloc */ export_calloc,
   /* premain */ {NULL, NULL, NULL, NULL},
   /* run_ctors_p */ 0,
    /* unused */ { 0, 0, 0},
   /* heapbase */ NULL, /* heapptr */ NULL, /* heaptop */ NULL,
   /* unused1 */ 0, /* forkee */ 0, /* hmodule */ NULL,
   /* api_major */ CYGWIN_VERSION_API_MAJOR,
   /* api_minor */ CYGWIN_VERSION_API_MINOR,
   /* unused2 */ {0, 0, 0, 0, 0},
   /* resourcelocks */ &_reslock, /* threadinterface */ &_mtinterf,
   /* impure_ptr */ &reent_data,
  };
};

char *old_title = NULL;
char title_buf[TITLESIZE + 1];

static void
do_global_dtors (void)
{
  if (user_data->dtors)
    {
      void (**pfunc)() = user_data->dtors;
      while (*++pfunc)
	(*pfunc) ();
    }
}

static void __stdcall
do_global_ctors (void (**in_pfunc)(), int force)
{
  if (!force)
    {
      if (user_data->forkee || user_data->run_ctors_p)
	return;		// inherit constructed stuff from parent pid
      user_data->run_ctors_p = 1;
    }

  /* Run ctors backwards, so skip the first entry and find how many
     there are, then run them. */

  void (**pfunc)() = in_pfunc;

  while (*++pfunc)
    ;
  while (--pfunc > in_pfunc)
    (*pfunc) ();

  if (user_data->magic_biscuit == SIZEOF_PER_PROCESS)
    atexit (do_global_dtors);
}

/* remember the type of Win32 OS being run for future use. */
os_type NO_COPY os_being_run;
char NO_COPY osname[40];

/* set_os_type: Set global variable os_being_run with type of Win32
   operating system being run.  This information is used internally
   to manage the inconsistency in Win32 API calls between Win32 OSes. */
/* Cygwin internal */
static void
set_os_type ()
{
  OSVERSIONINFO os_version_info;
  const char *os;

  memset (&os_version_info, 0, sizeof os_version_info);
  os_version_info.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
  GetVersionEx (&os_version_info);

  switch (os_version_info.dwPlatformId)
    {
      case VER_PLATFORM_WIN32_NT:
	os_being_run = winNT;
	os = "NT";
	break;
      case VER_PLATFORM_WIN32_WINDOWS:
	if (os_version_info.dwMinorVersion == 0)
	  {
	    os_being_run = win95;
	    os = "95";
	  }
	else /* os_version_info.dwMinorVersion == 10 */
	  {
	    os_being_run = win98;
	    os = "98";
	  }
	break;
      default:
	os_being_run = unknown;
	os = "??";
	break;
    }
  __small_sprintf (osname, "%s-%d.%d", os, os_version_info.dwMajorVersion,
		   os_version_info.dwMinorVersion);
}

host_dependent_constants NO_COPY host_dependent;

/* Constructor for host_dependent_constants.  */

void
host_dependent_constants::init ()
{
  extern DWORD chunksize;
  /* fhandler_disk_file::lock needs a platform specific upper word
     value for locking entire files.

     fhandler_base::open requires host dependent file sharing
     attributes.  */

  switch (os_being_run)
    {
    case winNT:
      win32_upper = 0xffffffff;
      shared = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
      break;

    case win98:
    case win95:
    case win32s:
      win32_upper = 0x00000000;
      shared = FILE_SHARE_READ | FILE_SHARE_WRITE;
      chunksize = 32 * 1024 * 1024;
      break;

    default:
      api_fatal ("unrecognized system type");
    }
}

/*
 * Replaces -@file in the command line with the contents of the file.
 * There may be multiple -@file's in a single command line
 * A \-@file is replaced with -@file so that echo \-@foo would print
 * -@foo and not the contents of foo.
 */
static int __stdcall
insert_file (char *name, char *&cmd)
{
  HANDLE f;
  DWORD size;

  f = CreateFile (name + 1,
		  GENERIC_READ,		 /* open for reading	*/
		  FILE_SHARE_READ,       /* share for reading	*/
		  &sec_none_nih,	 /* no security		*/
		  OPEN_EXISTING,	 /* existing file only	*/
		  FILE_ATTRIBUTE_NORMAL, /* normal file		*/
		  NULL);		 /* no attr. template	*/

  if (f == INVALID_HANDLE_VALUE)
    {
      debug_printf ("couldn't open file '%s', %E", name);
      return FALSE;
    }

  /* This only supports files up to about 4 billion bytes in
     size.  I am making the bold assumption that this is big
     enough for this feature */
  size = GetFileSize (f, NULL);
  if (size == 0xFFFFFFFF)
    {
      debug_printf ("couldn't get file size for '%s', %E", name);
      return FALSE;
    }

  int new_size = strlen (cmd) + size + 2;
  char *tmp = (char *) malloc (new_size);
  if (!tmp)
    {
      debug_printf ("malloc failed, %E");
      return FALSE;
    }

  /* realloc passed as it should */
  DWORD rf_read;
  BOOL rf_result;
  rf_result = ReadFile (f, tmp, size, &rf_read, NULL);
  CloseHandle (f);
  if (!rf_result || (rf_read != size))
    {
      debug_printf ("ReadFile failed, %E");
      return FALSE;
    }

  tmp[size++] = ' ';
  strcpy (tmp + size, cmd);
  cmd = tmp;
  return TRUE;
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
      if ((p = strchr (cmd, quote)) != NULL)
	strcpy (p, p + 1);
      return p + 1;
    }

  /* This must have been run from a Windows shell, so preserve
     quotes for globify to play with later. */
  while (*++cmd)
    if ((p = strpbrk (cmd, "\\\"")) == NULL)
      {
	cmd = strchr (cmd, '\0');	// no closing quote
	break;
      }
    else if (quote == '\'')
      continue;
    else if (*p == '\\')
      cmd = ++p;
    else if (p[1] == quote)
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
  if (!dos_spec && isquote(*word) && word[1] && word[2])
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
      debug_printf ("argv[%d] = '%s'\n", n++, *gv);
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
	  debug_printf ("argv[%d] = '%s'\n", argc, word);
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
    {
      api_fatal ("per_process sanity check failed");
    }

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
  if (p->api_major != cygwin_version.api_major)
    api_fatal ("cygwin DLL and APP are out of sync -- API version mismatch %d < %d",
	       p->api_major, cygwin_version.api_major);

  if (CYGWIN_VERSION_DLL_MAKE_COMBINED (p->dll_major, p->dll_minor) <=
      CYGWIN_VERSION_DLL_BAD_SIGNAL_MASK)
    signal_shift_subtract = 0;
}

static NO_COPY STARTUPINFO si;
# define fork_info ((struct child_info_fork *)(si.lpReserved2))
# define spawn_info ((struct child_info_spawn *)(si.lpReserved2))
child_info_fork NO_COPY *child_proc_info = NULL;
static MEMORY_BASIC_INFORMATION sm;

// __inline__ void
extern void
alloc_stack_hard_way (child_info_fork *ci, volatile char *b)
{
  void *new_stack_pointer;
  MEMORY_BASIC_INFORMATION m;
  void *newbase;
  int newlen;
  LPBYTE curbot = (LPBYTE) sm.BaseAddress + sm.RegionSize;
  bool noguard;

  if (ci->stacktop > (LPBYTE) sm.AllocationBase && ci->stacktop < curbot)
    {
      newbase = curbot;
      newlen = (LPBYTE) ci->stackbottom - (LPBYTE) curbot;
      noguard = 1;
    }
  else
    {
      newbase = ci->stacktop;
      newlen = (DWORD) ci->stackbottom - (DWORD) ci->stacktop;
      noguard = 0;
    }
  if (!VirtualAlloc (newbase, newlen, MEM_RESERVE, PAGE_NOACCESS))
    api_fatal ("fork: can't reserve memory for stack %p - %p, %E",
		ci->stacktop, ci->stackbottom);

  new_stack_pointer = (void *) ((LPBYTE) ci->stackbottom - ci->stacksize);

  if (!VirtualAlloc (new_stack_pointer, ci->stacksize, MEM_COMMIT,
		     PAGE_EXECUTE_READWRITE))
    api_fatal ("fork: can't commit memory for stack %p(%d), %E",
	       new_stack_pointer, ci->stacksize);
  if (!VirtualQuery ((LPCVOID) new_stack_pointer, &m, sizeof m))
    api_fatal ("fork: couldn't get new stack info, %E");
  if (!noguard)
    {
      m.BaseAddress = (LPVOID)((DWORD)m.BaseAddress - 1);
      if (!VirtualAlloc ((LPVOID) m.BaseAddress, 1, MEM_COMMIT,
			 PAGE_EXECUTE_READWRITE|PAGE_GUARD))
	api_fatal ("fork: couldn't allocate new stack guard page %p, %E",
		   m.BaseAddress);
    }
  if (!VirtualQuery ((LPCVOID) m.BaseAddress, &m, sizeof m))
    api_fatal ("fork: couldn't get new stack info, %E");
  ci->stacktop = m.BaseAddress;
  *b = 0;
}

/* extend the stack prior to fork longjmp */

static void
alloc_stack (child_info_fork *ci)
{
  /* FIXME: adding 16384 seems to avoid a stack copy problem during
     fork on Win95, but I don't know exactly why yet. DJ */
  volatile char b[ci->stacksize + 16384];

  if (ci->type == PROC_FORK)
    ci->stacksize = 0;		// flag to fork not to do any funny business
  else
    {
      if (!VirtualQuery ((LPCVOID) &b, &sm, sizeof sm))
	api_fatal ("fork: couldn't get stack info, %E");

      if (sm.AllocationBase != ci->stacktop)
	alloc_stack_hard_way (ci, b + sizeof(b) - 1);
      else
	ci->stacksize = 0;
    }

  return;
}

static NO_COPY int mypid = 0;
int _declspec(dllexport) __argc = 0;
char _declspec(dllexport) **__argv = NULL;

void
sigthread::init (const char *s)
{
  lock = new_muto (FALSE, s);
  id = GetCurrentThreadId ();
}

/* Take over from libc's crt0.o and start the application. Note the
   various special cases when Cygwin DLL is being runtime loaded (as
   opposed to being link-time loaded by Cygwin apps) from a non
   cygwin app via LoadLibrary.  */
static void
dll_crt0_1 ()
{
  /* According to onno@stack.urc.tue.nl, the exception handler record must
     be on the stack.  */
  /* FIXME: Verify forked children get their exception handler set up ok. */
  exception_list cygwin_except_entry;

  /* Initialize SIGSEGV handling, etc...  Because the exception handler
     references data in the shared area, this must be done after
     shared_init. */
  init_exceptions (&cygwin_except_entry);

  do_global_ctors (&__CTOR_LIST__, 1);

  /* Set the os_being_run global. */
  set_os_type ();
  check_sanity_and_sync (user_data);

  /* Nasty static stuff needed by newlib -- point to a local copy of
     the reent stuff.
     Note: this MUST be done here (before the forkee code) as the
     fork copy code doesn't copy the data in libccrt0.cc (that's why we
     pass in the per_process struct into the .dll from libccrt0). */

  _impure_ptr = &reent_data;

  user_data->resourcelocks->Init ();
  user_data->threadinterface->Init0 ();
  regthread ("main", GetCurrentThreadId ());

  char **envp = NULL;

  if (child_proc_info)
    {
      switch (child_proc_info->type)
	{
	  case PROC_FORK:
	  case PROC_FORK1:
	    alloc_stack (fork_info);
	    set_myself (mypid);
	    user_data->forkee = child_proc_info->cygpid;
	    user_data->heaptop = child_proc_info->heaptop;
	    user_data->heapbase = child_proc_info->heapbase;
	    user_data->heapptr = child_proc_info->heapptr;
	    ProtectHandle (child_proc_info->forker_finished);
	    break;
	  case PROC_EXEC:
	  case PROC_SPAWN:
	    HANDLE h;
	    cygheap = spawn_info->cygheap;
	    cygheap_max = spawn_info->cygheap_max;
	    cygheap_fixup_in_child (spawn_info->parent);
	    if (!spawn_info->moreinfo->myself_pinfo ||
		!DuplicateHandle (hMainProc, spawn_info->moreinfo->myself_pinfo,
				 hMainProc, &h, 0, 0,
				 DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE))
	      h = NULL;
	    set_myself (mypid, h);
	    __argc = spawn_info->moreinfo->argc;
	    __argv = spawn_info->moreinfo->argv;
	    envp = spawn_info->moreinfo->environ;
	    cygcwd.fixup_after_exec (spawn_info->moreinfo->cwd_win32,
				     spawn_info->moreinfo->cwd_posix,
				     spawn_info->moreinfo->cwd_hash);
	    fdtab.fixup_after_exec (spawn_info->parent, spawn_info->moreinfo->nfds,
				    spawn_info->moreinfo->fds);
	    CloseHandle (spawn_info->parent);
	    if (spawn_info->moreinfo->old_title)
	      {
		old_title = strcpy (title_buf, spawn_info->moreinfo->old_title);
		cfree (spawn_info->moreinfo->old_title);
	      }
	    ProtectHandle (child_proc_info->subproc_ready);
	    break;
	}
    }
  ProtectHandle (hMainProc);
  ProtectHandle (hMainThread);

  /* Initialize the host dependent constants object. */
  host_dependent.init ();

  /* Initialize the cygwin subsystem if this is the first process,
     or attach to the shared data structure if it's already running. */
  shared_init ();

  mainthread.init ("mainthread"); // For use in determining if signals
				  //  should be blocked.

  (void) SetErrorMode (SEM_FAILCRITICALERRORS);

  /* Initialize the heap. */
  heap_init ();

  /* Initialize events. */
  events_init ();

  threadname_init ();
  debug_init ();

  /* Allow backup semantics. It's better done only once on process start
     instead of each time a file is opened. */
  set_process_privileges ();

  if (user_data->forkee)
    {
      /* If we've played with the stack, stacksize != 0.  That means that
	 fork() was invoked from other than the main thread.  Make sure that
	 frame pointer is referencing the new stack so that the OS knows what
	 to do when it needs to increase the size of the stack.

	 NOTE: Don't do anything that involves the stack until you've completed
	 this step. */
      if (fork_info->stacksize)
	{
	  asm ("movl %0,%%fs:4" : : "r" (fork_info->stackbottom));
	  asm ("movl %0,%%fs:8" : : "r" (fork_info->stacktop));
	}

      longjmp (fork_info->jmp, fork_info->cygpid);
    }

  cygheap_init ();	/* Initialize cygwin's heap */
  cygcwd.init ();

  /* Initialize our process table entry. */
  pinfo_init (envp);

  if (!old_title && GetConsoleTitle (title_buf, TITLESIZE))
      old_title = title_buf;

  /* Nasty static stuff needed by newlib - initialize it.
     Note that impure_ptr has already been set up to point to this above
     NB. This *MUST* be done here, just after the forkee code as some
     of the calls below (eg. uinfo_init) do stdio calls - this area must
     be set to zero before then. */

  user_data->threadinterface->ClearReent();
  user_data->threadinterface->Init1();

  char *line = GetCommandLineA ();

  line = strcpy ((char *) alloca (strlen (line) + 1), line);

  /* Set new console title if appropriate. */

  if (display_title && !dynamically_loaded)
    {
      char *cp = line;
      if (strip_title_path)
	for (char *ptr = cp; *ptr && *ptr != ' '; ptr++)
	  if (isdirsep (*ptr))
	    cp = ptr + 1;
      set_console_title (cp);
    }

  /* Allocate fdtab */
  dtable_init ();

/* Initialize uid, gid. */
  uinfo_init ();

  /* Initialize signal/subprocess handling. */
  sigproc_init ();

  /* Connect to tty. */
  tty_init ();

  /* Set up standard fds in file descriptor table. */
  stdio_init ();

  if (!__argc)
    {
      /* Scan the command line and build argv.  Expand wildcards if not
	 called from another cygwin process. */
      build_argv (line, __argv, __argc,
		  NOTSTATE (myself, PID_CYGPARENT) && allow_glob);

      /* Convert argv[0] to posix rules if it's currently blatantly
	 win32 style. */
      if ((strchr (__argv[0], ':')) || (strchr (__argv[0], '\\')))
	{
	  char *new_argv0 = (char *) alloca (MAX_PATH);
	  cygwin_conv_to_posix_path (__argv[0], new_argv0);
	  char *p = strchr (new_argv0, '\0') - 4;
	  if (p > new_argv0 && strcasematch (p, ".exe"))
	    *p = '\0';
	  __argv[0] = new_argv0;
	}
    }

  if (user_data->premain[0])
    for (unsigned int i = 0; i < PREMAIN_LEN / 2; i++)
      user_data->premain[i] (__argc, __argv);

  /* Set up __progname for getopt error call. */
  __progname = __argv[0];

  cygwin_finished_initializing = 1;
  /* Call init of loaded dlls. */
  dlls.init ();

  /* Execute any specified "premain" functions */
  if (user_data->premain[PREMAIN_LEN / 2])
    for (unsigned int i = PREMAIN_LEN / 2; i < PREMAIN_LEN; i++)
      user_data->premain[i] (__argc, __argv);

  debug_printf ("user_data->main %p", user_data->main);

  if (dynamically_loaded)
    {
      set_errno (0);
      return;
    }

  /* Flush signals and ensure that signal thread is up and running. Can't
     do this for noncygwin case since the signal thread is blocked due to
     LoadLibrary serialization. */
  sig_send (NULL, __SIGFLUSH);

  set_errno (0);

  MALLOC_CHECK;
  if (user_data->main)
    exit (user_data->main (__argc, __argv, *user_data->envptr));
}

/* Wrap the real one, otherwise gdb gets confused about
   two symbols with the same name, but different addresses.

   UPTR is a pointer to global data that lives on the libc side of the
   line [if one distinguishes the application from the dll].  */

extern "C" void __stdcall
_dll_crt0 ()
{
  char zeros[sizeof (fork_info->zero)] = {0};

  /* Set the os_being_run global. */
  set_os_type ();

#ifdef DEBUGGING
  char buf[80];
  if (GetEnvironmentVariable ("CYGWIN_SLEEP", buf, sizeof (buf)))
    {
      small_printf ("Sleeping %d, pid %u\n", atoi (buf), GetCurrentProcessId ());
      Sleep (atoi(buf));
    }
#endif

  main_environ = user_data->envptr;
  user_data->heapbase = user_data->heapptr = user_data->heaptop = NULL;

  set_console_handler ();
  if (!DuplicateHandle (GetCurrentProcess (), GetCurrentProcess (),
		       GetCurrentProcess (), &hMainProc, 0, FALSE,
			DUPLICATE_SAME_ACCESS))
    hMainProc = GetCurrentProcess ();

  DuplicateHandle (hMainProc, GetCurrentThread (), hMainProc,
		   &hMainThread, 0, FALSE, DUPLICATE_SAME_ACCESS);

  GetStartupInfo (&si);
  if (si.cbReserved2 >= EXEC_MAGIC_SIZE &&
      memcmp (fork_info->zero, zeros, sizeof (zeros)) == 0)
    {
      switch (fork_info->type)
	{
	  case PROC_EXEC:
	  case PROC_SPAWN:
	  case PROC_FORK:
	  case PROC_FORK1:
	    {
	      child_proc_info = fork_info;
	      mypid = child_proc_info->cygpid;
	      cygwin_shared_h = child_proc_info->shared_h;
	      console_shared_h = child_proc_info->console_h;

	      /* We don't want subprocesses to inherit this */
	      if (dynamically_loaded)
		parent_alive = NULL;
	      else if (!DuplicateHandle (hMainProc, child_proc_info->parent_alive,
					hMainProc, &parent_alive, 0, 0,
					DUPLICATE_SAME_ACCESS
					| DUPLICATE_CLOSE_SOURCE))
		  system_printf ("parent_alive DuplicateHandle failed, %E");

	      break;
	    }
	  default:
	    if ((fork_info->type & PROC_MAGIC_MASK) == PROC_MAGIC_GENERIC)
	      api_fatal ("conflicting versions of cygwin1.dll detected.  Use only the most recent version.\n");
	}
    }
  dll_crt0_1 ();
}

void
dll_crt0 (per_process *uptr)
{
  /* Set the local copy of the pointer into the user space. */
  if (uptr && uptr != user_data)
    {
      memcpy (user_data, uptr, per_process_overwrite);
      *(user_data->impure_ptr_ptr) = &reent_data;
    }
  _dll_crt0 ();
}

/* This must be called by anyone who uses LoadLibrary to load cygwin1.dll */
extern "C" void
cygwin_dll_init ()
{
  static char **envp;
  static int _fmode;
  user_data->heapbase = user_data->heapptr = user_data->heaptop = NULL;

  if (!DuplicateHandle (GetCurrentProcess (), GetCurrentProcess (),
		       GetCurrentProcess (), &hMainProc, 0, FALSE,
			DUPLICATE_SAME_ACCESS))
    hMainProc = GetCurrentProcess ();

  DuplicateHandle (hMainProc, GetCurrentThread (), hMainProc,
		   &hMainThread, 0, FALSE, DUPLICATE_SAME_ACCESS);
  user_data->magic_biscuit = sizeof (per_process);

  user_data->envptr = &envp;
  user_data->fmode_ptr = &_fmode;

  dll_crt0_1 ();
}

extern "C" void
__main (void)
{
  do_global_ctors (user_data->ctors, FALSE);
}

enum
  {
    ES_SIGNAL = 1,
    ES_CLOSEALL = 2,
    ES_SIGPROCTERMINATE = 3
  };

extern "C" void __stdcall
do_exit (int status)
{
  BOOL cleanup_pinfo;
  UINT n = (UINT) status;
  static int NO_COPY exit_state = 0;

  syscall_printf ("do_exit (%d)", n);

  vfork_save *vf = vfork_storage.val ();
  if (vf != NULL && vf->pid < 0)
    {
      vf->pid = status < 0 ? status : -status;
      longjmp (vf->j, 1);
    }

  if (exit_state < ES_SIGNAL)
    {
      exit_state = ES_SIGNAL;
      if (!(n & EXIT_REPARENTING))
	{
	  signal (SIGCHLD, SIG_IGN);
	  signal (SIGHUP, SIG_IGN);
	  signal (SIGINT, SIG_IGN);
	  signal (SIGQUIT, SIG_IGN);
	}
    }

  if ((hExeced && hExeced != INVALID_HANDLE_VALUE) || (n & EXIT_NOCLOSEALL))
    n &= ~EXIT_NOCLOSEALL;
  else if (exit_state < ES_CLOSEALL)
    {
      exit_state = ES_CLOSEALL;
      close_all_files ();
    }

  if (exit_state < ES_SIGPROCTERMINATE)
    {
      exit_state = ES_SIGPROCTERMINATE;
      sigproc_terminate ();
    }

  if (n & EXIT_REPARENTING)
    {
      n &= ~EXIT_REPARENTING;
      cleanup_pinfo = FALSE;
    }
  else
    {
      myself->stopsig = 0;

      /* restore console title */
      if (old_title && display_title)
	set_console_title (old_title);

      /* Kill orphaned children on group leader exit */
      if (myself->pid == myself->pgid)
	{
	  sigproc_printf ("%d == pgrp %d, send SIG{HUP,CONT} to stopped children",
			  myself->pid, myself->pgid);
	  kill_pgrp (myself->pgid, -SIGHUP);
	}

      /* Kill the foreground process group on session leader exit */
      if (getpgrp () > 0 && myself->pid == myself->sid && tty_attached (myself))
	{
	  tty *tp = cygwin_shared->tty[myself->ctty];
	  sigproc_printf ("%d == sid %d, send SIGHUP to children",
			  myself->pid, myself->sid);

	  if (tp->getsid () == myself->sid)
	    kill (-tp->getpgid (), SIGHUP);
	}
      tty_terminate ();
      cleanup_pinfo = TRUE;
    }

  window_terminate ();
  fill_rusage (&myself->rusage_self, hMainProc);

  events_terminate ();

  if (hExeced && hExeced != INVALID_HANDLE_VALUE)
    {
      debug_printf ("Killing(%d) non-cygwin process, handle %p", n, hExeced);
      TerminateProcess (hExeced, n);
      ForceCloseHandle1 (hExeced, childhProc);
    }

  if (cleanup_pinfo)
    myself->record_death ();
  else
    sigproc_printf ("not cleanup_pinfo");

  shared_terminate ();

  sigproc_printf ("calling ExitProcess %d", n);
  minimal_printf ("winpid %d, exit %d", GetCurrentProcessId (), n);
  ExitProcess (n);
}

extern "C" void
_exit (int n)
{
  do_exit ((DWORD) n & 0xffff);
}

extern "C" void
__api_fatal (const char *fmt, ...)
{
  char buf[4096];
  va_list ap;

  va_start (ap, fmt);
  __small_vsprintf (buf, fmt, ap);
  va_end (ap);
  small_printf ("%s\n", buf);

  /* We are going down without mercy.  Make sure we reset
     our process_state. */
  sigproc_terminate ();
  myself->record_death ();
#ifdef DEBUGGING
  (void) try_to_debug ();
#endif
  ExitProcess (1);
}

extern "C" {

/* This struct is unused, but it illustrates the layout of a DLL
   information block. */
struct DLLinfo
{
  char jmpinst[4];
  HANDLE h;
  DWORD flag;
  char name[0];
};

/* FIXME: This is not thread-safe! */
__asm__ ("
msg1:
  .ascii \"couldn't dynamically determine load address for '%s' (handle %p), %E\\0\"

  .align 32
noload:
  popl %edx		# Get the address of the information block
  movl 8(%edx),%eax	# Should we 'ignore' the lack
  test $1,%eax		#  of this function?
  jz 1f			# Nope.
  decl %eax		# Yes.  This is the # of bytes + 1
  popl %edx		# Caller's caller
  addl %eax,%esp	# Pop off bytes
  xor %eax,%eax		# Zero functional return
  jmp *%edx		# Return
1:
  movl 4(%edx),%eax	# Handle value
  pushl (%eax)
  leal 12(%edx),%eax	# Location of name of function
  push %eax
  push $msg1		# The message
  call ___api_fatal	# Print message. Never returns

  .globl cygwin_dll_func_load
cygwin_dll_func_load:
  movl (%esp),%eax	# 'Return address' contains load info
  addl $12,%eax		# Address of name of function to load
  pushl %eax		# Second argument
  movl -8(%eax),%eax	# Address of Handle to DLL
  pushl (%eax)		# Handle to DLL
  call _GetProcAddress@8# Load it
  test %eax,%eax	# Success?
  jne gotit		# Yes
  jmp noload		# Issue an error or return
gotit:
  popl %ecx		# Pointer to 'return address'
  movb $0xe0,-1(%ecx)	# Turn preceding call to a jmp *%eax
  movl %eax,(%ecx)	# Point dispatch to address loaded above
  jmp *%eax
");

LoadDLLinitfunc (user32)
{
  HANDLE h;
  static NO_COPY LONG here = -1L;

  while (InterlockedIncrement (&here))
    {
      InterlockedDecrement (&here);
small_printf ("Multiple tries to read user32.dll\n");
      Sleep (0);
    }

  if (user32_handle)
    /* nothing to do */;
  else if ((h = LoadLibrary ("user32.dll")) != NULL)
    user32_handle = h;
  else if (!user32_handle)
    api_fatal ("could not load user32.dll, %E");

  InterlockedDecrement (&here);
  return 0;		/* Already done by another thread? */
}

LoadDLLinitfunc (advapi32)
{
  HANDLE h;
  static NO_COPY LONG here = -1L;

  while (InterlockedIncrement (&here))
    {
      InterlockedDecrement (&here);
small_printf ("Multiple tries to read advapi32.dll\n");
      Sleep (0);
    }

  if (advapi32_handle)
    /* nothing to do */;
  else if ((h = LoadLibrary ("advapi32.dll")) != NULL)
    advapi32_handle = h;
  else if (!advapi32_handle)
    api_fatal ("could not load advapi32.dll, %E");

  InterlockedDecrement (&here);
  return 0;
}

static void dummy_autoload (void) __attribute__ ((unused));
static void
dummy_autoload (void)
{
LoadDLLinit (user32)
LoadDLLfunc (CharToOemBuffA, 12, user32)
LoadDLLfunc (CreateWindowExA, 48, user32)
LoadDLLfunc (DefWindowProcA, 16, user32)
LoadDLLfunc (DispatchMessageA, 4, user32)
LoadDLLfunc (FindWindowA, 8, user32)
LoadDLLfunc (GetMessageA, 16, user32)
LoadDLLfunc (GetProcessWindowStation, 0, user32)
LoadDLLfunc (GetThreadDesktop, 4, user32)
LoadDLLfunc (GetUserObjectInformationA, 20, user32)
LoadDLLfunc (KillTimer, 8, user32)
LoadDLLfunc (MessageBoxA, 16, user32)
LoadDLLfunc (MsgWaitForMultipleObjects, 20, user32)
LoadDLLfunc (OemToCharBuffA, 12, user32)
LoadDLLfunc (PeekMessageA, 20, user32)
LoadDLLfunc (PostMessageA, 16, user32)
LoadDLLfunc (PostQuitMessage, 4, user32)
LoadDLLfunc (RegisterClassA, 4, user32)
LoadDLLfunc (SendMessageA, 16, user32)
LoadDLLfunc (SetTimer, 16, user32)
LoadDLLfunc (SetUserObjectSecurity, 12, user32)

LoadDLLinit (advapi32)
LoadDLLfunc (AddAccessAllowedAce, 16, advapi32)
LoadDLLfunc (AddAccessDeniedAce, 16, advapi32)
LoadDLLfunc (AddAce, 20, advapi32)
LoadDLLfunc (AdjustTokenPrivileges, 24, advapi32)
LoadDLLfunc (CopySid, 12, advapi32)
LoadDLLfunc (CreateProcessAsUserA, 44, advapi32)
LoadDLLfuncEx (CryptAcquireContextA, 20, advapi32, 1)
LoadDLLfuncEx (CryptGenRandom, 12, advapi32, 1)
LoadDLLfuncEx (CryptReleaseContext, 8, advapi32, 1)
LoadDLLfunc (DeregisterEventSource, 4, advapi32)
LoadDLLfunc (EqualSid, 8, advapi32)
LoadDLLfunc (GetAce, 12, advapi32)
LoadDLLfunc (GetFileSecurityA, 20, advapi32)
LoadDLLfunc (GetLengthSid, 4, advapi32)
LoadDLLfunc (GetSecurityDescriptorDacl, 16, advapi32)
LoadDLLfunc (GetSecurityDescriptorGroup, 12, advapi32)
LoadDLLfunc (GetSecurityDescriptorOwner, 12, advapi32)
LoadDLLfunc (GetSidIdentifierAuthority, 4, advapi32)
LoadDLLfunc (GetSidSubAuthority, 8, advapi32)
LoadDLLfunc (GetSidSubAuthorityCount, 4, advapi32)
LoadDLLfunc (GetTokenInformation, 20, advapi32)
LoadDLLfunc (GetUserNameA, 8, advapi32)
LoadDLLfunc (ImpersonateLoggedOnUser, 4, advapi32)
LoadDLLfunc (InitializeAcl, 12, advapi32)
LoadDLLfunc (InitializeSecurityDescriptor, 8, advapi32)
LoadDLLfunc (InitializeSid, 12, advapi32)
LoadDLLfunc (IsValidSid, 4, advapi32)
LoadDLLfunc (LogonUserA, 24, advapi32)
LoadDLLfunc (LookupAccountNameA, 28, advapi32)
LoadDLLfunc (LookupAccountSidA, 28, advapi32)
LoadDLLfunc (LookupPrivilegeValueA, 12, advapi32)
LoadDLLfunc (MakeSelfRelativeSD, 12, advapi32)
LoadDLLfunc (OpenProcessToken, 12, advapi32)
LoadDLLfunc (RegCloseKey, 4, advapi32)
LoadDLLfunc (RegCreateKeyExA, 36, advapi32)
LoadDLLfunc (RegDeleteKeyA, 8, advapi32)
LoadDLLfunc (RegDeleteValueA, 8, advapi32)
LoadDLLfunc (RegLoadKeyA, 12, advapi32)
LoadDLLfunc (RegEnumKeyExA, 32, advapi32)
LoadDLLfunc (RegOpenKeyExA, 20, advapi32)
LoadDLLfunc (RegQueryValueExA, 24, advapi32)
LoadDLLfunc (RegSetValueExA, 24, advapi32)
LoadDLLfunc (RegisterEventSourceA, 8, advapi32)
LoadDLLfunc (ReportEventA, 36, advapi32)
LoadDLLfunc (RevertToSelf, 0, advapi32)
LoadDLLfunc (SetKernelObjectSecurity, 12, advapi32)
LoadDLLfunc (SetSecurityDescriptorDacl, 16, advapi32)
LoadDLLfunc (SetSecurityDescriptorGroup, 12, advapi32)
LoadDLLfunc (SetSecurityDescriptorOwner, 12, advapi32)
}
}
