/* dcrt0.cc -- essentially the main() for the Cygwin dll

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
#include <sys/param.h>
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
#include <fenv.h>
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
      CloseHandle (f);
      debug_printf ("couldn't get file size for '%s', %E", name);
      return false;
    }

  int new_size = strlen (cmd) + size + 2;
  char *tmp = (char *) malloc (new_size);
  if (!tmp)
    {
      CloseHandle (f);
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
      free (tmp);
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
      if (*(p = strchrnul (cmd, quote)))
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

/* Either X:[...] or \\server\[...] */
#define is_dos_path(s) (isdrive(s) \
			|| ((s)[0] == '\\' \
			    && (s)[1] == '\\' \
			    && isalpha ((s)[2]) \
			    && strchr ((s) + 3, '\\')))

static int __stdcall
globify (char *word, char **&argv, int &argc, int &argvlen)
{
  if (*word != '~' && strpbrk (word, "?*[\"\'(){}") == NULL)
    return 0;

  int n = 0;
  char *p, *s;
  int dos_spec = is_dos_path (word);
  if (!dos_spec && isquote (*word) && word[1] && word[2])
    dos_spec = is_dos_path (word + 1);

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
	      /* Handle quoting.  Only strip off quotes if the parent is
		 a Cygwin process, or if the word starts with a '@'.
		 In this case, the insert_file function needs an unquoted
		 DOS filename and globbing isn't performed anyway. */
	      cmd = quoted (cmd, winshell && argc > 0 && *word != '@');
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

  if (argv)
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
    api_fatal ("cygwin DLL and APP are out of sync -- DLL version mismatch %u < %u",
	       p->dll_major, CYGWIN_VERSION_DLL_EPOCH);

  /* magic_biscuit != 0 if using the old style version numbering scheme.  */
  if (p->magic_biscuit != SIZEOF_PER_PROCESS)
    api_fatal ("Incompatible cygwin .dll -- incompatible per_process info %u != %u",
	       p->magic_biscuit, SIZEOF_PER_PROCESS);

  /* Complain if incompatible API changes made */
  if (p->api_major > cygwin_version.api_major)
    api_fatal ("cygwin DLL and APP are out of sync -- API version mismatch %u > %u",
	       p->api_major, cygwin_version.api_major);

#ifdef __i386__
  /* This is a kludge to work around a version of _cygwin_common_crt0
     which overwrote the cxx_malloc field with the local DLL copy.
     Hilarity ensues if the DLL is not loaded while the process
     is forking. */
  __cygwin_user_data.cxx_malloc = &default_cygwin_cxx_malloc;
#endif
}

child_info NO_COPY *child_proc_info;

/* Extend the stack prior to fork longjmp. */
void
child_info_fork::alloc_stack ()
{
  PTEB teb = NtCurrentTeb ();
  if (teb->Tib.StackBase != stackbase)
    {
      void *stack_ptr;
      size_t stacksize;

      /* If guardsize is -1, we have been started from a pthread with an
	 application-provided stack, and the stack has just to be used as is. */
      if (guardsize == (size_t) -1)
	return;
      /* Reserve entire stack. */
      stacksize = (PBYTE) stackbase - (PBYTE) stackaddr;
      if (!VirtualAlloc (stackaddr, stacksize, MEM_RESERVE, PAGE_NOACCESS))
	{
	  api_fatal ("fork: can't reserve memory for parent stack "
		     "%p - %p, (child has %p - %p), %E",
		     stackaddr, stackbase, teb->DeallocationStack,
		     teb->Tib.StackBase);
	}
      /* Commit the area commited in parent. */
      stacksize = (PBYTE) stackbase - (PBYTE) stacklimit;
      stack_ptr = VirtualAlloc (stacklimit, stacksize, MEM_COMMIT,
				PAGE_READWRITE);
      if (!stack_ptr)
	api_fatal ("can't commit memory for stack %p(%ly), %E",
		   stacklimit, stacksize);
      /* Set up guardpages. */
      ULONG real_guardsize = guardsize
			     ? roundup2 (guardsize, wincap.page_size ())
			     : wincap.def_guard_page_size ();
      if (stack_ptr > stackaddr)
	{
	  stack_ptr = (void *) ((PBYTE) stack_ptr - real_guardsize);
	  if (!VirtualAlloc (stack_ptr, real_guardsize, MEM_COMMIT,
			     PAGE_READWRITE | PAGE_GUARD))
	    api_fatal ("fork: couldn't allocate new stack guard page %p, %E",
		       stack_ptr);
	}
      /* Set thread stack guarantee matching the guardsize.
	 Note that the guardsize is one page bigger than the guarantee. */
      if (real_guardsize > wincap.def_guard_page_size ())
	{
	  real_guardsize -= wincap.page_size ();
	  SetThreadStackGuarantee (&real_guardsize);
	}
    }
  else
    {
      /* Fork has been called from main thread.  Simply commit the region
	 of the stack commited in the parent but not yet commited in the
	 child and create new guardpages. */
      if (NtCurrentTeb ()->Tib.StackLimit > stacklimit)
	{
	  SIZE_T commitsize = (PBYTE) NtCurrentTeb ()->Tib.StackLimit
			      - (PBYTE) stacklimit;
	  if (!VirtualAlloc (stacklimit, commitsize, MEM_COMMIT, PAGE_READWRITE))
	    api_fatal ("can't commit child memory for stack %p(%ly), %E",
		       stacklimit, commitsize);
	  PVOID guardpage = (PBYTE) stacklimit - wincap.def_guard_page_size ();
	  if (!VirtualAlloc (guardpage, wincap.def_guard_page_size (),
			     MEM_COMMIT, PAGE_READWRITE | PAGE_GUARD))
	    api_fatal ("fork: couldn't allocate new stack guard page %p, %E",
		       guardpage);
	  NtCurrentTeb ()->Tib.StackLimit = stacklimit;
	}
      /* This only affects forked children of a process started from a native
	 64 bit process, but it doesn't hurt to do it unconditionally.  Fix
	 StackBase in the child to be the same as in the parent, so that the
	 computation of _my_tls is correct. */
      teb->Tib.StackBase = (PVOID) stackbase;
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
  char buf[PATH_MAX];
  if (GetEnvironmentVariableA ("CYGWIN_DEBUG", buf, sizeof (buf) - 1))
    {
      char buf1[PATH_MAX];
      GetModuleFileName (NULL, buf1, PATH_MAX);
      char *p = strpbrk (buf, ":=");
      if (!p)
	p = (char *) "gdb.exe -nw";
      else
	*p++ = '\0';
      if (strcasestr (buf1, buf))
	{
	  extern PWCHAR debugger_command;

	  debugger_command = (PWCHAR) HeapAlloc (GetProcessHeap (), 0,
						 (2 * NT_MAX_PATH + 20)
						 * sizeof (WCHAR));
	  if (!debugger_command)
	    return;
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
	multiple_cygwin_problem ("cygheap base", (uintptr_t) res->cygheap,
				 (uintptr_t) &_cygheap_start);

      unsigned should_be_cb = 0;
      switch (res->type)
	{
	  case _CH_FORK:
	    in_forkee = true;
	    should_be_cb = sizeof (child_info_fork);
	    fallthrough;
	  case _CH_SPAWN:
	  case _CH_EXEC:
	    if (!should_be_cb)
	      should_be_cb = sizeof (child_info_spawn);
	    if (should_be_cb != res->cb)
	      multiple_cygwin_problem ("proc size", res->cb, should_be_cb);
	    else if (sizeof (fhandler_union) != res->fhandler_union_cb)
	      multiple_cygwin_problem ("fhandler size", res->fhandler_union_cb,
				       sizeof (fhandler_union));
	    if (res->isstraced ())
	      {
		while (!being_debugged ())
		  yield ();
		strace.activate (res->type == _CH_FORK);
	      }
	    break;
	  default:
	    system_printf ("unknown exec type %u", res->type);
	    fallthrough;
	  case _CH_WHOOPS:
	    res = NULL;
	    break;
	}
    }

  return res;
}

#ifdef __x86_64__
#define dll_data_start &__data_start__
#define dll_data_end &__data_end__
#define dll_bss_start &__bss_start__
#define dll_bss_end &__bss_end__
#else
#define dll_data_start &_data_start__
#define dll_data_end &_data_end__
#define dll_bss_start &_bss_start__
#define dll_bss_end &_bss_end__
#endif

void
child_info_fork::handle_fork ()
{
  cygheap_fixup_in_child (false);
  memory_init ();
  myself.thisproc (NULL);
  myself->uid = cygheap->user.real_uid;
  myself->gid = cygheap->user.real_gid;

  child_copy (parent, false, silentfail (),
	      "dll data", dll_data_start, dll_data_end,
	      "dll bss", dll_bss_start, dll_bss_end,
	      "user heap", cygheap->user_heap.base, cygheap->user_heap.ptr,
	      NULL);

  /* If my_wr_proc_pipe != NULL then it's a leftover handle from a previously
     forked process.  Close it now or suffer confusion with the parent of our
     parent.  */
  if (my_wr_proc_pipe)
    ForceCloseHandle1 (my_wr_proc_pipe, wr_proc_pipe);

  /* Setup our write end of the process pipe.  Clear the one in the structure.
     The destructor should never be called for this but, it can't hurt to be
     safe. */
  my_wr_proc_pipe = wr_proc_pipe;
  rd_proc_pipe = wr_proc_pipe = NULL;
  /* Do the relocations here.  These will actually likely be overwritten by the
     below child_copy but we do them here in case there is a read-only section
     which does not get copied by fork. */
  _pei386_runtime_relocator (user_data);

  /* step 2 now that the dll has its heap filled in, we can fill in the
     user's data and bss since user_data is now filled out. */
  child_copy (parent, false, silentfail (),
	      "data", user_data->data_start, user_data->data_end,
	      "bss", user_data->bss_start, user_data->bss_end,
	      NULL);

  if (fixup_mmaps_after_fork (parent))
    api_fatal ("recreate_mmaps_after_fork_failed");

  /* We need to occupy the address space for dynamically loaded dlls
     before we allocate any dynamic object, or we may end up with
     error "address space needed by <dll> is already occupied"
     for no good reason (seen with some relocated dll). */
  dlls.reserve_space ();
}

bool
child_info_spawn::get_parent_handle ()
{
  parent = OpenProcess (PROCESS_VM_READ, FALSE, parent_winpid);
  moreinfo->myself_pinfo = NULL;
  return !!parent;
}

void
child_info_spawn::handle_spawn ()
{
  extern void fixup_lockf_after_exec (bool);
  HANDLE h = INVALID_HANDLE_VALUE;
  if (!dynamically_loaded || get_parent_handle ())
      {
	cygheap_fixup_in_child (true);
	memory_init ();
      }

  cygheap->pid = cygpid;

  /* Spawned process sets h to INVALID_HANDLE_VALUE to notify
     pinfo::thisproc not to create another pid. */
  if (!moreinfo->myself_pinfo ||
      !DuplicateHandle (GetCurrentProcess (), moreinfo->myself_pinfo,
			GetCurrentProcess (), &h, 0,
			FALSE, DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE))
    h = (type == _CH_SPAWN) ? INVALID_HANDLE_VALUE : NULL;

  /* Setup our write end of the process pipe.  Clear the one in the structure.
     The destructor should never be called for this but, it can't hurt to be
     safe. */
  my_wr_proc_pipe = wr_proc_pipe;
  rd_proc_pipe = wr_proc_pipe = NULL;

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

  if (child_proc_info->parent)
    {
      /* We no longer need this handle so close it.  Need to do
	 this after debug_fixup_after_fork_exec or DEBUGGING handling of
	 handles might get confused. */
      CloseHandle (child_proc_info->parent);
      child_proc_info->parent = NULL;
    }

  signal_fixup_after_exec ();
  fixup_lockf_after_exec (type == _CH_EXEC);
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
      /* We need the Windows dir with NT prefix in path.cc.  Note that we
	 don't append a backslash, because we need the dir without backslash
	 for the environment. */
      wcpcpy (windows_directory_buf, L"\\??\\");
      windows_directory_length =
	    GetSystemWindowsDirectoryW (windows_directory, MAX_PATH - 4);
      RtlInitCountedUnicodeString (&windows_directory_path,
	    windows_directory_buf,
	    (windows_directory_length + 4) * sizeof (WCHAR));
#ifdef __i386__
      system_wow64_directory_length =
	    GetSystemWow64DirectoryW (system_wow64_directory, MAX_PATH);
      if (system_wow64_directory_length)
	{
	  system_wow64_directory[system_wow64_directory_length++] = L'\\';
	  system_wow64_directory[system_wow64_directory_length] = L'\0';
	}
#endif /* __i386__ */
    }
}

void
dll_crt0_0 ()
{
  wincap.init ();
  GetModuleFileNameW (NULL, global_progname, NT_MAX_PATH);
  child_proc_info = get_cygwin_startup_info ();
  init_windows_system_directory ();
  initial_env ();

  SetErrorMode (SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);

  lock_process::init ();
  _impure_ptr = _GLOBAL_REENT;
  _impure_ptr->_stdin = &_impure_ptr->__sf[0];
  _impure_ptr->_stdout = &_impure_ptr->__sf[1];
  _impure_ptr->_stderr = &_impure_ptr->__sf[2];
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
      setup_cygheap ();
      memory_init ();
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

  _main_tls = &_my_tls;

  /* Initialize signal processing here, early, in the hopes that the creation
     of a thread early in the process will cause more predictability in memory
     layout for the main thread. */
  if (!dynamically_loaded)
    sigproc_init ();

#ifdef __x86_64__
  /* See comment preceeding myfault_altstack_handler in exception.cc. */
  AddVectoredContinueHandler (0, myfault_altstack_handler);
#endif

  debug_printf ("finished dll_crt0_0 initialization");
}

static inline void
main_thread_sinit ()
{
  __sinit (_impure_ptr);
  /* At this point, _impure_ptr == _global_impure_ptr == _GLOBAL_REENT is
     initialized, but _REENT == _my_tls.local_clib doesn't know about it.
     It has been copied over from _GLOBAL_REENT in _cygtls::init_thread
     *before* the initialization took place.

     As soon as the main thread calls a stdio function, this would be
     rectified.  But if another thread calls a stdio function on
     stdin/out/err before the main thread does, all the required
     initialization of stdin/out/err will be done, but _REENT->__sdidinit
     is *still* 0.  This in turn will result in a call to __sinit in the
     wrong spot.  The input or output buffer will be NULLed and nothing is
     read or written in the first stdio function call in the main thread.

     To fix this issue we have to copy over the relevant part of _GLOBAL_REENT
     to _REENT here again. */
  _REENT->__sdidinit = -1;
  _REENT->__cleanup = _GLOBAL_REENT->__cleanup;
}

/* Take over from libc's crt0.o and start the application. Note the
   various special cases when Cygwin DLL is being runtime loaded (as
   opposed to being link-time loaded by Cygwin apps) from a non
   cygwin app via LoadLibrary.  */
void
dll_crt0_1 (void *)
{
  extern void initial_setlocale ();

  _my_tls.incyg++;
  /* Inherit "parent" exec'ed process sigmask */
  if (spawn_info && !in_forkee)
    _my_tls.sigmask = spawn_info->moreinfo->sigmask;

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
	  small_printf ("total allocated %y\n", (i - 1) * n);
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

  /* Initialize debug muto, if DLL is built with --enable-debugging.
     Need to do this before any helper threads start. */
  debug_init ();

  cygbench ("pre-forkee");
  if (in_forkee)
    {
      /* Make sure to restore the TEB's stack info.  If guardsize is -1 the
	 stack has been provided by the application and must not be deallocated
	 automagically when the thread exits.

	 NOTE: Don't do anything that involves the stack until you've completed
	 this step. */
      PTEB teb = NtCurrentTeb ();
      teb->Tib.StackBase = (PVOID) fork_info->stackbase;
      teb->Tib.StackLimit = (PVOID) fork_info->stacklimit;
      teb->DeallocationStack = (fork_info->guardsize == (size_t) -1)
			       ? NULL
			       : (PVOID) fork_info->stackaddr;

      /* Not resetting _my_tls.incyg here because presumably fork will overwrite
	 it with the value of the forker and all will be good.   */
      longjmp (fork_info->jmp, true);
    }

  main_thread_sinit ();

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

  /* Set internal locale to the environment settings. */
  initial_setlocale ();

  uinfo_init ();	/* initialize user info */

  /* Connect to tty. */
  tty::init_session ();

  if (!__argc)
    {
      PWCHAR wline = GetCommandLineW ();
      size_t size = sys_wcstombs_no_path (NULL, 0, wline) + 1;
      char *line = (char *) alloca (size);
      sys_wcstombs_no_path (line, size, wline);

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
  SetThreadName (GetCurrentThreadId (), program_invocation_short_name);

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

  cygbench (__progname);

  ld_preload ();
  /* Per POSIX set the default application locale back to "C". */
  _setlocale_r (_REENT, LC_CTYPE, "C");

  if (!user_data->main)
    {
      /* Handle any signals which may have arrived */
      _my_tls.call_signal_handler ();
      _my_tls.incyg--;	/* Not in Cygwin anymore */
    }
  else
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
      /* Handle any signals which may have arrived */
      sig_dispatch_pending (false);
      _my_tls.call_signal_handler ();
      _my_tls.incyg--;	/* Not in Cygwin anymore */
#ifdef __x86_64__
      cygwin_exit (user_data->main (__argc, newargv, __cygwin_environ));
#else
      cygwin_exit (user_data->main (__argc, newargv, *user_data->envptr));
#endif
    }
  __asm__ ("				\n\
	.global _cygwin_exit_return	\n\
	.global __cygwin_exit_return	\n\
_cygwin_exit_return:			\n\
__cygwin_exit_return:			\n\
		nop			\n\
");
}

extern "C" void __stdcall
_dll_crt0 ()
{
#ifdef __x86_64__
  /* Starting with Windows 10 rel 1511, the main stack of an application is
     not reproducible if a 64 bit process has been started from a 32 bit
     process.  Given that we have enough virtual address space on 64 bit
     anyway, we now always move the main thread stack to the stack area
     reserved for pthread stacks.  This allows a reproducible stack space
     under our own control and avoids collision with the OS. */
  if (!dynamically_loaded)
    {
      if (!in_forkee)
	{
	  /* Must be static since it's referenced after the stack and frame
	     pointer registers have been changed. */
	  static PVOID allocationbase;
	  SIZE_T commitsize = in_forkee ? (PBYTE) fork_info->stackbase
					  - (PBYTE) fork_info->stacklimit
					: 0;
	  PVOID stackaddr = create_new_main_thread_stack (allocationbase,
							  commitsize);
	  if (stackaddr)
	    {
	      /* Set stack pointer to new address.  Set frame pointer to
	         stack pointer and subtract 32 bytes for shadow space. */
	      __asm__ ("\n\
		       movq %[ADDR], %%rsp \n\
		       movq  %%rsp, %%rbp  \n\
		       subq  $32,%%rsp     \n"
		       : : [ADDR] "r" (stackaddr));
	      /* We're on the new stack now.  Free up space taken by the former
		 main thread stack and set DeallocationStack correctly. */
	      VirtualFree (NtCurrentTeb ()->DeallocationStack, 0, MEM_RELEASE);
	      NtCurrentTeb ()->DeallocationStack = allocationbase;
	    }
	}
      else
	fork_info->alloc_stack ();
    }
#else
  main_environ = user_data->envptr;
  if (in_forkee)
    fork_info->alloc_stack ();
#endif

  fesetenv (FE_DFL_ENV);
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
#ifdef __i386__
  static char **envp;
#endif
  static int _fmode;

  user_data->magic_biscuit = sizeof (per_process);

#ifdef __i386__
  user_data->envptr = &envp;
#endif
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

void __reg1
do_exit (int status)
{
  syscall_printf ("do_exit (%d), exit_state %d", status, exit_state);

  lock_process until_exit (true);

  if (exit_state < ES_EVENTS_TERMINATE)
    exit_state = ES_EVENTS_TERMINATE;

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
	  sigproc_printf ("%u == pgrp %u, send SIG{HUP,CONT} to stopped children",
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
	  sigproc_printf ("%u == sid %u, send SIGHUP to children",
			  myself->pid, myself->sid);

	/* CGF FIXME: This can't be right. */
	  if (tp->getsid () == myself->sid)
	    tp->kill_pgrp (SIGHUP);
	}

    }

  myself.exit (n);
}

/* When introducing support for -fuse-cxa-atexit with Cygwin 1.7.32 and
   GCC 4.8.3-3, we defined __dso_value as &ImageBase.  This supposedly allowed
   a reproducible value which could also be easily evaluated in cygwin_atexit.
   However, when building C++ applications with -fuse-cxa-atexit, G++ creates
   calls to __cxa_atexit using the *address* of __dso_handle as DSO handle.

   So what we do here is this:  A call to __cxa_atexit from the application
   actually calls cygwin__cxa_atexit.  From dso_handle (which is either
   &__dso_handle, or __dso_handle == ImageBase or NULL) we fetch the dll
   structure of the DLL.  Then use dll::handle == ImageBase as the actual DSO
   handle value in calls to __cxa_atexit and __cxa_finalize.
   Thus, __cxa_atexit becomes entirely independent of the incoming value of
   dso_handle, as long as it's *some* pointer into the DSO's address space. */
extern "C" int
cygwin__cxa_atexit (void (*fn)(void *), void *obj, void *dso_handle)
{
  dll *d = dso_handle ? dlls.find (dso_handle) : NULL;
  return __cxa_atexit (fn, obj, d ? d->handle : NULL);
}

/* This function is only called for applications built with Cygwin versions
   up to API 0.279.  Starting with API 0.280 (Cygwin 1.7.33/1.8.6-2), atexit
   is a statically linked function inside of libcygwin.a.  The reason is that
   the old method to fetch the caller return address is unreliable given GCCs
   ability to perform tail call elimination.  For the details, see the below
   comment.  The atexit replacement is defined in libcygwin.a to allow reliable
   access to the correct DSO handle. */
extern "C" int
cygwin_atexit (void (*fn) (void))
{
  int res;

  dll *d = dlls.find ((void *) _my_tls.retaddr ());
#ifdef __x86_64__
  /* x86_64 DLLs created with GCC 4.8.3-3 register __gcc_deregister_frame
     as atexit function using a call to atexit, rather than __cxa_atexit.
     Due to GCC's tail call optimizing, cygwin_atexit doesn't get the correct
     return address on the stack.  As a result it fails to get the HMODULE of
     the caller and thus calls atexit rather than __cxa_atexit.  Then, if the
     module gets dlclosed, __cxa_finalize (called from dll_list::detach) can't
     remove __gcc_deregister_frame from the atexit function chain.  So at
     process exit, __call_exitprocs calls __gcc_deregister_frame while the
     module is already unloaded and the __gcc_deregister_frame function not
     available ==> SEGV.

     This also occurs for other functions.

     Workaround: If dlls.find fails, try to find the dll entry of the DLL
     containing fn.  If that works, proceed by calling __cxa_atexit, otherwise
     call atexit.

     This *should* be sufficiently safe.  Ultimately, new applications will
     use the statically linked atexit function though, as outlined above. */
  if (!d)
    d = dlls.find ((void *) fn);
#endif
  res = d ? __cxa_atexit ((void (*) (void *)) fn, NULL, d->handle) : atexit (fn);
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
multiple_cygwin_problem (const char *what, uintptr_t magic_version, uintptr_t version)
{
  if (_cygwin_testing && (strstr (what, "proc") || strstr (what, "cygheap")))
    {
      child_proc_info->type = _CH_WHOOPS;
      return;
    }

  if (GetEnvironmentVariableA ("CYGWIN_MISMATCH_OK", NULL, 0))
    return;

  if (CYGWIN_VERSION_MAGIC_VERSION (magic_version) == version)
    system_printf ("%s magic number mismatch detected - %p/%ly", what, magic_version, version);
  else
    api_fatal ("%s mismatch detected - %ly/%ly.\n\
This problem is probably due to using incompatible versions of the cygwin DLL.\n\
Search for cygwin1.dll using the Windows Start->Find/Search facility\n\
and delete all but the most recent version.  The most recent version *should*\n\
reside in x:\\cygwin\\bin, where 'x' is the drive on which you have\n\
installed the cygwin distribution.  Rebooting is also suggested if you\n\
are unable to find another cygwin DLL.",
	       what, magic_version, version);
}

#ifdef DEBUGGING
void __reg1
cygbench (const char *s)
{
  if (GetEnvironmentVariableA ("CYGWIN_BENCH", NULL, 0))
    small_printf ("%05u ***** %s : %10d\n", GetCurrentProcessId (), s, strace.microseconds ());
}
#endif
