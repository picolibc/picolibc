/* cygtls.cc

   Copyright 2003, 2004 Red Hat, Inc.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "thread.h"
#include "cygtls.h"
#include "assert.h"
#include <syslog.h>
#include <signal.h>
#include "exceptions.h"
#include "sync.h"
#include "cygerrno.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "cygthread.h"

class sentry
{
  static muto *lock;
  int destroy;
public:
  void init ();
  bool acquired () {return lock->acquired ();}
  sentry () {destroy = 0;}
  sentry (DWORD wait) {destroy = lock->acquire (wait);}
  ~sentry () {if (destroy) lock->release ();}
  friend void _threadinfo::init ();
};

muto NO_COPY *sentry::lock;

static size_t NO_COPY nthreads;

#define THREADLIST_CHUNK 256

void
_threadinfo::init ()
{
  if (cygheap->threadlist)
    memset (cygheap->threadlist, 0, cygheap->sthreads * sizeof (cygheap->threadlist[0]));
  else
    {
      cygheap->sthreads = THREADLIST_CHUNK;
      cygheap->threadlist = (_threadinfo **) ccalloc (HEAP_TLS, cygheap->sthreads,
						     sizeof (cygheap->threadlist[0]));
    }
  new_muto1 (sentry::lock, sentry_lock);
}

void
_threadinfo::set_state (bool is_exception)
{
  initialized = CYGTLS_INITIALIZED + is_exception;
}

void
_threadinfo::reset_exception ()
{
  if (initialized == CYGTLS_EXCEPTION)
    {
#ifdef DEBUGGING
      debug_printf ("resetting stack after an exception stack %p, stackptr %p", stack, stackptr);
#endif
      set_state (false);
    }
}

/* Two calls to get the stack right... */
void
_threadinfo::call (DWORD (*func) (void *, void *), void *arg)
{
  char buf[CYGTLS_PADSIZE];
  call2 (func, arg, buf);
}

void
_threadinfo::call2 (DWORD (*func) (void *, void *), void *arg, void *buf)
{
  exception_list except_entry;
  /* Initialize this thread's ability to respond to things like
     SIGSEGV or SIGFPE. */
  init_exceptions (&except_entry);
  _my_tls.init_thread (buf, func);
  DWORD res = func (arg, buf);
  _my_tls.remove (INFINITE);
  ExitThread (res);
}

void
_threadinfo::init_thread (void *x, DWORD (*func) (void *, void *))
{
  if (x)
    {
      memset (this, 0, ((char *) padding - (char *) this));
      stackptr = stack;
      if (_GLOBAL_REENT)
	{
	  local_clib._stdin = _GLOBAL_REENT->_stdin;
	  local_clib._stdout = _GLOBAL_REENT->_stdout;
	  local_clib._stderr = _GLOBAL_REENT->_stderr;
	  local_clib.__sdidinit = _GLOBAL_REENT->__sdidinit;
	  local_clib.__cleanup = _GLOBAL_REENT->__cleanup;
	}
      local_clib._current_locale = "C";
      locals.process_logmask = LOG_UPTO (LOG_DEBUG);
    }

  set_state (false);
  errno_addr = &(local_clib._errno);

  if ((void *) func == (void *) cygthread::stub
      || (void *) func == (void *) cygthread::simplestub)
    return;

  sentry here (INFINITE);
  if (nthreads < cygheap->sthreads)
    {
      cygheap->threadlist = (_threadinfo **)
	crealloc (cygheap->threadlist, (cygheap->sthreads += THREADLIST_CHUNK)
		 * sizeof (cygheap->threadlist[0]));
      memset (cygheap->threadlist + nthreads, 0, THREADLIST_CHUNK * sizeof (cygheap->threadlist[0]));
    }

  cygheap->threadlist[nthreads++] = this;
}

void
_threadinfo::remove (DWORD wait)
{
  sentry here (wait);
  if (here.acquired ())
    {
      for (size_t i = 0; i < nthreads; i++)
	if (&_my_tls == cygheap->threadlist[i])
	  {
	    if (i < --nthreads)
	      cygheap->threadlist[i] = cygheap->threadlist[nthreads];
	    break;
	  }
    }
}

void
_threadinfo::push (__stack_t addr, bool exception)
{
  *stackptr++ = (__stack_t) addr;
  set_state (exception);
}

__stack_t
_threadinfo::pop ()
{
#ifdef DEBUGGING
  assert (stackptr > stack);
#endif
  __stack_t res = *--stackptr;
#ifdef DEBUGGING
  *stackptr = 0;
  debug_printf ("popped %p, stack %p, stackptr %p", res, stack, stackptr);
#endif
  return res;
}

#define BAD_IX ((size_t) -1)
static size_t NO_COPY threadlist_ix = BAD_IX;

_threadinfo *
_threadinfo::find_tls (int sig)
{
  sentry here (INFINITE);
  __asm__ volatile (".equ _threadlist_exception_return,.");
  _threadinfo *res = NULL;
  for (threadlist_ix = 0; threadlist_ix < nthreads; threadlist_ix++)
    if (sigismember (&(cygheap->threadlist[threadlist_ix]->sigwait_mask), sig))
      {
	res = cygheap->threadlist[threadlist_ix];
	break;
      }
  threadlist_ix = BAD_IX;
  return res;
}

extern "C" DWORD __stdcall RtlUnwind (void *, void *, void *, DWORD);
static int
handle_threadlist_exception (EXCEPTION_RECORD *e, void *frame, CONTEXT *, void *)
{
  small_printf ("in handle_threadlist_exception!\n");
  if (e->ExceptionCode != STATUS_ACCESS_VIOLATION)
    return 1;

  sentry here;
  if (threadlist_ix != BAD_IX || !here.acquired ())
    return 1;

  extern void *threadlist_exception_return;
  cygheap->threadlist[threadlist_ix]->remove (INFINITE);
  threadlist_ix = 0;
  RtlUnwind (frame, threadlist_exception_return, e, 0);
  return 0;
}

void
_threadinfo::init_threadlist_exceptions (exception_list *el)
{
  extern void init_exception_handler (exception_list *, exception_handler *);
  init_exception_handler (el, handle_threadlist_exception);
}
