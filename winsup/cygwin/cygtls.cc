/* cygtls.cc

   Copyright 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012,
   2013 Red Hat, Inc.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#define USE_SYS_TYPES_FD_SET
#include "cygtls.h"
#include <syslog.h>
#include <stdlib.h>
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "sigproc.h"
#include "exception.h"

/* Two calls to get the stack right... */
void
_cygtls::call (DWORD (*func) (void *, void *), void *arg)
{
  char buf[CYGTLS_PADSIZE];
  /* Initialize this thread's ability to respond to things like
     SIGSEGV or SIGFPE. */
  exception protect;
  _my_tls.call2 (func, arg, buf);
}

static int
dll_cmp (const void *a, const void *b)
{
  return wcscasecmp ((const wchar_t *) a, *(const wchar_t **) b);
}

/* Keep sorted!
   This is a list of well-known core system DLLs which contain code
   whiuch is started in its own thread by the system.  Kernel32.dll,
   for instance, contains the thread called on every Ctrl-C keypress
   in a console window.  The DLLs in this list are not recognized as
   BLODAs. */
const wchar_t *well_known_dlls[] =
{
  L"advapi32.dll",
  L"kernel32.dll",
  L"kernelbase.dll",
  L"mswsock.dll",
  L"ntdll.dll",
  L"ole32.dll",
  L"shlwapi.dll",
  L"wbemprox.dll",
  L"ws2_32.dll",
};

void
_cygtls::call2 (DWORD (*func) (void *, void *), void *arg, void *buf)
{
  init_thread (buf, func);

  /* Optional BLODA detection.  The idea is that the function address is
     supposed to be within Cygwin itself.  This is also true for pthreads,
     since pthreads are always calling thread_wrapper in miscfuncs.cc.
     Therefore, every function call to a function outside of the Cygwin DLL
     is potentially a thread injected into the Cygwin process by some BLODA.

     But that's a bit too simple.  Assuming the application itself calls
     CreateThread, then this is a bad idea, but not really invalid.  So we
     shouldn't print a BLODA message if the address is within the loaded
     image of the application.  Also, ntdll.dll starts threads into the
     application which */
  if (detect_bloda)
    {
      PIMAGE_DOS_HEADER img_start = (PIMAGE_DOS_HEADER) GetModuleHandle (NULL);
      PIMAGE_NT_HEADERS32 ntheader = (PIMAGE_NT_HEADERS32)
			       ((PBYTE) img_start + img_start->e_lfanew);
      void *img_end = (void *) ((PBYTE) img_start
				+ ntheader->OptionalHeader.SizeOfImage);
      if (((void *) func < (void *) cygwin_hmodule
	   || (void *) func > (void *) cygheap)
	  && ((void *) func < (void *) img_start || (void *) func >= img_end))
	{
	  MEMORY_BASIC_INFORMATION mbi;
	  wchar_t modname[PATH_MAX];

	  VirtualQuery ((PVOID) func, &mbi, sizeof mbi);
	  GetModuleFileNameW ((HMODULE) mbi.AllocationBase, modname, PATH_MAX);
	  /* Fetch basename and check against list of above system DLLs. */
	  const wchar_t *modbasename = wcsrchr (modname, L'\\') + 1;
	  if (!bsearch (modbasename, well_known_dlls,
			sizeof well_known_dlls / sizeof well_known_dlls[0],
			sizeof well_known_dlls[0], dll_cmp))
	    small_printf ("\n\nPotential BLODA detected!  Thread function "
			  "called outside of Cygwin DLL:\n  %W\n\n", modname);
	}
    }

  DWORD res = func (arg, buf);
  remove (INFINITE);
  /* Don't call ExitThread on the main thread since we may have been
     dynamically loaded.  */
  if ((void *) func != (void *) dll_crt0_1
      && (void *) func != (void *) dll_dllcrt0_1)
    ExitThread (res);
}

void
_cygtls::init_thread (void *x, DWORD (*func) (void *, void *))
{
  if (x)
    {
      memset (this, 0, sizeof (*this));
      _REENT_INIT_PTR (&local_clib);
      stackptr = stack;
      if (_GLOBAL_REENT)
	{
	  local_clib._stdin = _GLOBAL_REENT->_stdin;
	  local_clib._stdout = _GLOBAL_REENT->_stdout;
	  local_clib._stderr = _GLOBAL_REENT->_stderr;
	  local_clib.__sdidinit = _GLOBAL_REENT->__sdidinit ? -1 : 0;
	  local_clib.__cleanup = _GLOBAL_REENT->__cleanup;
	  local_clib.__sglue._niobs = 3;
	  local_clib.__sglue._iobs = &_GLOBAL_REENT->__sf[0];
	}
    }

  thread_id = GetCurrentThreadId ();
  initialized = CYGTLS_INITIALIZED;
  errno_addr = &(local_clib._errno);
  locals.cw_timer = NULL;

  if ((void *) func == (void *) cygthread::stub
      || (void *) func == (void *) cygthread::simplestub)
    return;

  cygheap->add_tls (this);
}

void
_cygtls::fixup_after_fork ()
{
  if (sig)
    {
      pop ();
      sig = 0;
    }
  stacklock = spinning = 0;
  signal_arrived = NULL;
  locals.select.sockevt = NULL;
  locals.cw_timer = NULL;
  wq.thread_ev = NULL;
}

#define free_local(x) \
  if (locals.x) \
    { \
      free (locals.x); \
      locals.x = NULL; \
    }

void
_cygtls::remove (DWORD wait)
{
  initialized = 0;
  if (exit_state >= ES_FINAL)
    return;

  debug_printf ("wait %u", wait);

  /* FIXME: Need some sort of atthreadexit function to allow things like
     select to control this themselves. */

  if (signal_arrived)
    {
      HANDLE h = signal_arrived;
      signal_arrived = NULL;
      CloseHandle (h);
    }

  /* Close handle and free memory used by select. */
  if (locals.select.sockevt)
    {
      CloseHandle (locals.select.sockevt);
      locals.select.sockevt = NULL;
      free_local (select.ser_num);
      free_local (select.w4);
    }
  /* Free memory used by network functions. */
  free_local (ntoa_buf);
  free_local (protoent_buf);
  free_local (servent_buf);
  free_local (hostent_buf);
  /* Free temporary TLS path buffers. */
  locals.pathbufs.destroy ();
  /* Close timer handle. */
  if (locals.cw_timer)
    NtClose (locals.cw_timer);
  cygheap->remove_tls (this, wait);
  remove_wq (wait);
}
