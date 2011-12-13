/* dcrt0.cc -- essentially the main() for the Cygwin dll

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006,
   2007, 2008, 2009, 2010, 2011 Red Hat, Inc.

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
#include <locale.h>
#include "environ.h"
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
#include "exception.h"
#include "cygxdr.h"
#include "fenv.h"
#include "ntdll.h"

#define MAX_AT_FILE_LEVEL 10

#define PREMAIN_LEN (sizeof (user_data->premain) / sizeof (user_data->premain[0]))

extern "C" void cygwin_exit (int) __attribute__ ((noreturn));
extern "C" void __sinit (_reent *);

static int NO_COPY envc;
static char NO_COPY **envp;

bool NO_COPY jit_debug;

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
		   GENERIC_READ,		/* open for reading	*/
		   FILE_SHARE_VALID_FLAGS,      /* share for reading	*/
		   &sec_none_nih,		/* default security	*/
		   OPEN_EXISTING,		/* existing file only	*/
		   FILE_ATTRIBUTE_NORMAL,	/* normal file		*/
		   NULL);			/* no attr. template	*/

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
	    size_t cnt = isascii (*s) ? 1 : mbtowc (NULL, s, MB_CUR_MAX);
	    if (cnt <= 1 || cnt == (size_t)-1)
	      *p++ = *s;
	    else
	      {
		--s;
		while (cnt-- > 0)
		  *p++ = *++s;
	      }
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

  /* This is a kludge to work around a version of _cygwin_common_crt0
     which overwrote the cxx_malloc field with the local DLL copy.
     Hilarity ensues if the DLL is not loaded while the process
     is forking. */
  __cygwin_user_data.cxx_malloc = &default_cygwin_cxx_malloc;
}

child_info NO_COPY *child_proc_info = NULL;

#define CYGWIN_GUARD (PAGE_EXECUTE_READWRITE | PAGE_GUARD)

void
child_info_fork::alloc_stack_hard_way (volatile char *b)
{
  void *stack_ptr;
  DWORD stacksize;

  /* First check if the requested stack area is part of the user heap
     or part of a mmapped region.  If so, we have been started from a
     pthread with an application-provided stack, and the stack has just
     to be used as is. */
  if ((stacktop >= cygheap->user_heap.base
      && stackbottom <= cygheap->user_heap.max)
      || is_mmapped_region ((caddr_t) stacktop, (caddr_t) stackbottom))
    return;

  /* First, try to reserve the entire stack. */
  stacksize = (char *) stackbottom - (char *) stackaddr;
  if (!VirtualAlloc (stackaddr, stacksize, MEM_RESERVE, PAGE_NOACCESS))
    api_fatal ("fork: can't reserve memory for stack %p - %p, %E",
	       stackaddr, stackbottom);
  stacksize = (char *) stackbottom - (char *) stacktop;
  stack_ptr = VirtualAlloc (stacktop, stacksize, MEM_COMMIT,
			    PAGE_EXECUTE_READWRITE);
  if (!stack_ptr)
    abort ("can't commit memory for stack %p(%d), %E", stacktop, stacksize);
  if (guardsize != (size_t) -1)
    {
      /* Allocate PAGE_GUARD page if it still fits. */
      if (stack_ptr > stackaddr)
	{
	  stack_ptr = (void *) ((LPBYTE) stack_ptr
					- wincap.page_size ());
	  if (!VirtualAlloc (stack_ptr, wincap.page_size (), MEM_COMMIT,
			     CYGWIN_GUARD))
	    api_fatal ("fork: couldn't allocate new stack guard page %p, %E",
		       stack_ptr);
	}
      /* Allocate POSIX guard pages. */
      if (guardsize > 0)
	VirtualAlloc (stackaddr, guardsize, MEM_COMMIT, PAGE_NOACCESS);
    }
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
      stackaddr = 0;
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
  DWORD len;
  char buf[NT_MAX_PATH];
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
	  jit_debug = true;
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
    {
      strace.activate (false);
      res = NULL;
    }
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
	  case _CH_FORK:
	    in_forkee = true;
	    should_be_cb = sizeof (child_info_fork);
	    /* fall through */;
	  case _CH_SPAWN:
	  case _CH_EXEC:
	    if (!should_be_cb)
	      should_be_cb = sizeof (child_info_spawn);
	    if (should_be_cb != res->cb)
	      multiple_cygwin_problem ("proc size", res->cb, should_be_cb);
	    else if (sizeof (fhandler_union) != res->fhandler_union_cb)
	      multiple_cygwin_problem ("fhandler size", res->fhandler_union_cb, sizeof (fhandler_union));
	    if (res->isstraced ())
	      {
		while (!being_debugged ())
		  yield ();
		strace.activate (res->type == _CH_FORK);
	      }
	    break;
	  default:
	    system_printf ("unknown exec type %d", res->type);
	    /* intentionally fall through */
	  case _CH_WHOOPS:
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
  memory_init (false);
  myself.thisproc (NULL);
  myself->uid = cygheap->user.real_uid;
  myself->gid = cygheap->user.real_gid;

  child_copy (parent, false,
	      "dll data", dll_data_start, dll_data_end,
	      "dll bss", dll_bss_start, dll_bss_end,
	      "user heap", cygheap->user_heap.base, cygheap->user_heap.ptr,
	      NULL);

  /* Do the relocations here.  These will actually likely be overwritten by the
     below child_copy but we do them here in case there is a read-only section
     which does not get copied by fork. */
  _pei386_runtime_relocator (user_data);

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
  memory_init (false);
  if (!moreinfo->myself_pinfo ||
      !DuplicateHandle (GetCurrentProcess (), moreinfo->myself_pinfo,
			GetCurrentProcess (), &h, 0,
			FALSE, DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE))
    h = NULL;
  myself.thisproc (h);
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

  /* If we're execing we may have "inherited" a list of children forked by the
     previous process executing under this pid.  Reattach them here so that we
     can wait for them.  */
  if (type == _CH_EXEC)
    reattach_children ();

  ready (true);

  /* Need to do this after debug_fixup_after_fork_exec or DEBUGGING handling of
     handles might get confused. */
  CloseHandle (child_proc_info->parent);
  child_proc_info->parent = NULL;

  signal_fixup_after_exec ();
  fixup_lockf_after_exec ();
}

/* Retrieve and store system directory for later use.  Note that the
   directory is stored with a trailing backslash! */
static void
init_windows_system_directory ()
{
  if (!windows_system_directory_length)
    {
      windows_system_directory_length =
	    GetSystemDirectoryW (windows_system_directory, MAX_PATH);
      if (windows_system_directory_length == 0)
	api_fatal ("can't find windows system directory");
      windows_system_directory[windows_system_directory_length++] = L'\\';
      windows_system_directory[windows_system_directory_length] = L'\0';

      system_wow64_directory_length =
	GetSystemWow64DirectoryW (system_wow64_directory, MAX_PATH);
      if (system_wow64_directory_length)
	{
	  system_wow64_directory[system_wow64_directory_length++] = L'\\';
	  system_wow64_directory[system_wow64_directory_length] = L'\0';
	}
    }
}

static bool NO_COPY wow64_respawn = false;

inline static bool
wow64_started_from_native64 ()
{
  /* On Windows XP 64 and 2003 64 there's a problem with processes running
     under WOW64.  The first process started from a 64 bit process has an
     unusual stack address for the main thread.  That is, an address which
     is in the usual space occupied by the process image, but below the auto
     load address of DLLs.  If we encounter this situation, check if we
     really have been started from a 64 bit process here.  If so, we exit
     early from dll_crt0_0 and respawn first thing in dll_crt0_1.  This
     ping-pong game is necessary to workaround a problem observed on
     Windows 2003 R2 64.  Starting with Cygwin 1.7.10 we don't link against
     advapi32.dll anymore.  However, *any* process linked against advapi32,
     directly or indirectly, now fails to respawn if respawn_wow_64_process
     is called during DLL_PROCESS_ATTACH initialization.  In that case
     CreateProcessW returns with ERROR_ACCESS_DENIED for some reason.
     Calling CreateProcessW later, inside dll_crt0_1 and so outside of
     dll initialization works as before, though. */
  NTSTATUS ret;
  PROCESS_BASIC_INFORMATION pbi;
  HANDLE parent;

  ULONG wow64 = TRUE;   /* Opt on the safe side. */

  /* Unfortunately there's no simpler way to retrieve the
     parent process in NT, as far as I know.  Hints welcome. */
  ret = NtQueryInformationProcess (NtCurrentProcess (),
                                   ProcessBasicInformation,
                                   &pbi, sizeof pbi, NULL);
  if (NT_SUCCESS (ret)
      && (parent = OpenProcess (PROCESS_QUERY_INFORMATION,
                                FALSE,
                                pbi.InheritedFromUniqueProcessId)))
    {
      NtQueryInformationProcess (parent, ProcessWow64Information,
                                 &wow64, sizeof wow64, NULL);
      CloseHandle (parent);
    }
  return !wow64;
}

inline static void
respawn_wow64_process ()
{
  /* The parent is a real 64 bit process.  Respawn. */
  WCHAR path[PATH_MAX];
  PROCESS_INFORMATION pi;
  STARTUPINFOW si;
  DWORD ret = 0;

  GetModuleFileNameW (NULL, path, PATH_MAX);
  GetStartupInfoW (&si);
  if (!CreateProcessW (path, GetCommandLineW (), NULL, NULL, TRUE,
		       CREATE_DEFAULT_ERROR_MODE
		       | GetPriorityClass (GetCurrentProcess ()),
		       NULL, NULL, &si, &pi))
    api_fatal ("Failed to create process <%W> <%W>, %E",
	       path, GetCommandLineW ());
  CloseHandle (pi.hThread);
  if (WaitForSingleObject (pi.hProcess, INFINITE) == WAIT_FAILED)
    api_fatal ("Waiting for process %d failed, %E", pi.dwProcessId);
  GetExitCodeProcess (pi.hProcess, &ret);
  CloseHandle (pi.hProcess);
  TerminateProcess (GetCurrentProcess (), ret);
  ExitProcess (ret);
}

void
dll_crt0_0 ()
{
  child_proc_info = get_cygwin_startup_info ();
  init_windows_system_directory ();
  init_global_security ();
  initial_env ();

  SetErrorMode (SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);

  lock_process::init ();
  _impure_ptr = _GLOBAL_REENT;
  _impure_ptr->_stdin = &_impure_ptr->__sf[0];
  _impure_ptr->_stdout = &_impure_ptr->__sf[1];
  _impure_ptr->_stderr = &_impure_ptr->__sf[2];
  _impure_ptr->_current_locale = "C";
  user_data->impure_ptr = _impure_ptr;
  user_data->impure_ptr_ptr = &_impure_ptr;

  DuplicateHandle (GetCurrentProcess (), GetCurrentThread (),
		   GetCurrentProcess (), &hMainThread,
		   0, false, DUPLICATE_SAME_ACCESS);

  NtOpenProcessToken (NtCurrentProcess (), MAXIMUM_ALLOWED, &hProcToken);
  set_cygwin_privileges (hProcToken);

  device::init ();
  do_global_ctors (&__CTOR_LIST__, 1);
  cygthread::init ();

  if (!child_proc_info)
    {
      memory_init (true);
      /* WOW64 bit process with stack at unusual address?  Check if we
	 have been started from 64 bit process ans set wow64_respawn.
	 Full description in wow64_started_from_native64 above. */
      BOOL wow64_test_stack_marker;
      if (wincap.is_wow64 ()
	  && &wow64_test_stack_marker >= (PBOOL) 0x400000
	  && &wow64_test_stack_marker <= (PBOOL) 0x10000000
	  && (wow64_respawn = wow64_started_from_native64 ()))
	return;
    }
  else
    {
      cygwin_user_h = child_proc_info->user_h;
      switch (child_proc_info->type)
	{
	  case _CH_FORK:
	    fork_info->handle_fork ();
	    break;
	  case _CH_SPAWN:
	  case _CH_EXEC:
	    spawn_info->handle_spawn ();
	    break;
	}
    }

  user_data->threadinterface->Init ();

  _cygtls::init ();

  /* Initialize events */
  events_init ();
  tty_list::init_session ();

  _main_tls = &_my_tls;

  /* Initialize signal processing here, early, in the hopes that the creation
     of a thread early in the process will cause more predictability in memory
     layout for the main thread. */
  if (!dynamically_loaded)
    sigproc_init ();

  debug_printf ("finished dll_crt0_0 initialization");
}

/* Take over from libc's crt0.o and start the application. Note the
   various special cases when Cygwin DLL is being runtime loaded (as
   opposed to being link-time loaded by Cygwin apps) from a non
   cygwin app via LoadLibrary.  */
void
dll_crt0_1 (void *)
{
  extern void initial_setlocale ();

  if (dynamically_loaded)
    sigproc_init ();

  check_sanity_and_sync (user_data);

  /* Initialize malloc and then call user_shared_initialize since it relies
     on a functioning malloc and it's possible that the user's program may
     have overridden malloc.  We only know about that at this stage,
     unfortunately. */
  malloc_init ();
  user_shared->initialize ();

#ifdef CYGHEAP_DEBUG
  int i = 0;
  const int n = 2 * 1024 * 1024;
  while (i--)
    {
      void *p = cmalloc (HEAP_STR, n);
      if (p)
	small_printf ("cmalloc returns %p\n", cmalloc (HEAP_STR, n));
      else
	{
	  small_printf ("total allocated %p\n", (i - 1) * n);
	  break;
	}
    }
#endif

  ProtectHandle (hMainThread);

  cygheap->cwd.init ();

  /* Initialize pthread mainthread when not forked and it is safe to call new,
     otherwise it is reinitalized in fixup_after_fork */
  if (!in_forkee)
    {
      pthread::init_mainthread ();
      _pei386_runtime_relocator (user_data);
    }

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
      if (fork_info->stackaddr)
	{
	  _tlsbase = (char *) fork_info->stackbottom;
	  _tlstop = (char *) fork_info->stacktop;
	}

      longjmp (fork_info->jmp, true);
    }

  __sinit (_impure_ptr);

#ifdef DEBUGGING
  {
  extern void fork_init ();
  fork_init ();
  }
#endif
  pinfo_init (envp, envc);
  strace.dll_info ();

  /* Allocate cygheap->fdtab */
  dtable_init ();

  uinfo_init ();	/* initialize user info */

  /* Connect to tty. */
  tty::init_session ();

  /* Set internal locale to the environment settings. */
  initial_setlocale ();

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
  program_invocation_name = __argv[0];
  program_invocation_short_name = __progname;
  if (__progname)
    {
      char *cp = strchr (__progname, '\0') - 4;
      if (cp > __progname && ascii_strcasematch (cp, ".exe"))
	*cp = '\0';
    }

  (void) xdr_set_vprintf (&cygxdr_vwarnx);
  cygwin_finished_initializing = true;
  /* Call init of loaded dlls. */
  dlls.init ();

  /* Execute any specified "premain" functions */
  if (user_data->premain[PREMAIN_LEN / 2])
    for (unsigned int i = PREMAIN_LEN / 2; i < PREMAIN_LEN; i++)
      user_data->premain[i] (__argc, __argv, user_data);

  set_errno (0);

  if (dynamically_loaded)
    {
      _setlocale_r (_REENT, LC_CTYPE, "C");
      return;
    }

  /* Disable case-insensitive globbing */
  ignore_case_with_glob = false;

  MALLOC_CHECK;
  cygbench (__progname);

  ld_preload ();
  /* Per POSIX set the default application locale back to "C". */
  _setlocale_r (_REENT, LC_CTYPE, "C");

  if (user_data->main)
    {
      /* Create a copy of Cygwin's version of __argv so that, if the user makes
	 a change to an element of argv[] it does not affect Cygwin's argv.
	 Changing the the contents of what argv[n] points to will still
	 affect Cygwin.  This is similar (but not exactly like) Linux. */
      char *newargv[__argc + 1];
      char **nav = newargv;
      char **oav = __argv;
      while ((*nav++ = *oav++) != NULL)
	continue;
      cygwin_exit (user_data->main (__argc, newargv, *user_data->envptr));
    }
  __asm__ ("				\n\
	.global __cygwin_exit_return	\n\
__cygwin_exit_return:			\n\
");
}

extern "C" void __stdcall
_dll_crt0 ()
{
  /* Respawn WOW64 process started from native 64 bit process.  See comment
     in wow64_started_from_native64 above for a full description. */
  if (wow64_respawn)
    respawn_wow64_process ();
#ifdef __i386__
  _feinitialise ();
#endif
  main_environ = user_data->envptr;
  if (in_forkee)
    {
      fork_info->alloc_stack ();
      _main_tls = &_my_tls;
    }

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
  /* Ordering is critical here.  DLL ctors have already been
     run as they were being loaded, so we should stack the
     queued call to DLL dtors now.  */
  atexit (dll_global_dtors);
  do_global_ctors (user_data->ctors, false);
  /* Now we have run global ctors, register their dtors.

     At exit, global dtors will run first, so the app can still
     use shared library functions while terminating; then the
     DLLs will be destroyed; finally newlib will shut down stdio
     and terminate itself.  */
  atexit (do_global_dtors);
  sig_dispatch_pending (true);
}

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

  if (exit_state < ES_EVENTS_TERMINATE)
    {
      exit_state = ES_EVENTS_TERMINATE;
      events_terminate ();
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

  UINT n = (UINT) status;
  if (exit_state < ES_THREADTERM)
    {
      exit_state = ES_THREADTERM;
      cygthread::terminate ();
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

  myself.exit (n);
}

extern "C" int
cygwin_atexit (void (*fn) (void))
{
  int res;
  dll *d = dlls.find ((void *) _my_tls.retaddr ());
  res = d ? __cxa_atexit ((void (*) (void *)) fn, NULL, d) : atexit (fn);
  return res;
}

extern "C" void
cygwin_exit (int n)
{
  exit_state = ES_EXIT_STARTING;
  exit (n);
}

extern "C" void
_exit (int n)
{
  do_exit (((DWORD) n & 0xff) << 8);
}

extern "C" void cygwin_stackdump ();

extern "C" void
vapi_fatal (const char *fmt, va_list ap)
{
  char buf[4096];
  int n = __small_sprintf (buf, "%P: *** fatal error %s- ", in_forkee ? "in forked process " : "");
  __small_vsprintf (buf + n, fmt, ap);
  va_end (ap);
  strace.prntf (_STRACE_SYSTEM, NULL, "%s", buf);

#ifdef DEBUGGING
  try_to_debug ();
#endif
  cygwin_stackdump ();
  myself.exit (__api_fatal_exit_val);
}

extern "C" void
api_fatal (const char *fmt, ...)
{
  va_list ap;

  va_start (ap, fmt);
  vapi_fatal (fmt, ap);
}

void
multiple_cygwin_problem (const char *what, unsigned magic_version, unsigned version)
{
  if (_cygwin_testing && (strstr (what, "proc") || strstr (what, "cygheap")))
    {
      child_proc_info->type = _CH_WHOOPS;
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
