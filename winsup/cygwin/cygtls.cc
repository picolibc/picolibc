/* cygtls.cc

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

void
_cygtls::call2 (DWORD (*func) (void *, void *), void *arg, void *buf)
{
  /* If func is pthread_wrapper, the final stack hasn't been set up yet.
     This only happens in pthread_wrapper itself.  Thus it doesn't make
     sense to call init_thread or perform BLODA detection.  pthread_wrapper
     eventually calls init_thread by itself. */
  if ((void *) func != (void *) pthread_wrapper)
    init_thread (buf, func);

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
      altstack.ss_flags = SS_DISABLE;
      if (_GLOBAL_REENT)
	{
	  local_clib._stdin = _GLOBAL_REENT->_stdin;
	  local_clib._stdout = _GLOBAL_REENT->_stdout;
	  local_clib._stderr = _GLOBAL_REENT->_stderr;
	  if (_GLOBAL_REENT->__cleanup)
	    local_clib.__cleanup = _cygtls::cleanup_early;
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

  HANDLE mutex = cygheap->remove_tls (this);
  remove_wq (wait);

  /* FIXME: Need some sort of atthreadexit function to allow things like
     select to control this themselves. */

  remove_pending_sigs ();
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
  if (mutex)
    {
      ReleaseMutex (mutex);
      CloseHandle (mutex);
    }
}

void
_cygtls::cleanup_early (struct _reent *)
{
  /* Do nothing */
}

#ifdef __x86_64__
void san::leave ()
{
  /* Restore tls_pathbuf counters in case of error. */
  _my_tls.locals.pathbufs._counters = _cnt;
  _my_tls.andreas = _clemente;
}
#endif
