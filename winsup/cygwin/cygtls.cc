/* cygtls.cc

   Copyright 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011 Red Hat, Inc.

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

class sentry
{
  static muto lock;
  int destroy;
public:
  void init ();
  bool acquired () {return lock.acquired ();}
  sentry () {destroy = 0;}
  sentry (DWORD wait) {destroy = lock.acquire (wait);}
  ~sentry () {if (destroy) lock.release ();}
  friend void _cygtls::init ();
};

muto NO_COPY sentry::lock;

static size_t NO_COPY nthreads;

#define THREADLIST_CHUNK 256

void
_cygtls::init ()
{
  if (cygheap->threadlist)
    memset (cygheap->threadlist, 0, cygheap->sthreads * sizeof (cygheap->threadlist[0]));
  else
    {
      cygheap->sthreads = THREADLIST_CHUNK;
      cygheap->threadlist = (_cygtls **) ccalloc_abort (HEAP_TLS, cygheap->sthreads,
							sizeof (cygheap->threadlist[0]));
    }
  sentry::lock.init ("sentry_lock");
}

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

  cygheap->user.reimpersonate ();

  sentry here (INFINITE);
  if (nthreads >= cygheap->sthreads)
    {
      cygheap->threadlist = (_cygtls **)
	crealloc_abort (cygheap->threadlist, (cygheap->sthreads += THREADLIST_CHUNK)
			* sizeof (cygheap->threadlist[0]));
      memset (cygheap->threadlist + nthreads, 0, THREADLIST_CHUNK * sizeof (cygheap->threadlist[0]));
    }

  cygheap->threadlist[nthreads++] = this;
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

  debug_printf ("wait %p", wait);

  /* FIXME: Need some sort of atthreadexit function to allow things like
     select to control this themselves. */

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

  do
    {
      sentry here (wait);
      if (here.acquired ())
	{
	  for (size_t i = 0; i < nthreads; i++)
	    if (this == cygheap->threadlist[i])
	      {
		if (i < --nthreads)
		  cygheap->threadlist[i] = cygheap->threadlist[nthreads];
		debug_printf ("removed %p element %d", this, i);
		break;
	      }
	}
    } while (0);
  remove_wq (wait);
}

void
_cygtls::push (__stack_t addr)
{
  *stackptr++ = (__stack_t) addr;
}


_cygtls *
_cygtls::find_tls (int sig)
{
  static int NO_COPY threadlist_ix;

  debug_printf ("sig %d\n", sig);
  sentry here (INFINITE);

  _cygtls *res = NULL;
  threadlist_ix = -1;

  myfault efault;
  if (efault.faulted ())
    cygheap->threadlist[threadlist_ix]->remove (INFINITE);

  while (++threadlist_ix < (int) nthreads)
    if (sigismember (&(cygheap->threadlist[threadlist_ix]->sigwait_mask), sig))
      {
	res = cygheap->threadlist[threadlist_ix];
	break;
      }
  return res;
}

void
_cygtls::set_siginfo (sigpacket *pack)
{
  infodata = pack->si;
}
