/* cygthread.cc

   Copyright 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005 Red Hat, Inc.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <windows.h>
#include <stdlib.h>
#include "exceptions.h"
#include "security.h"
#include "sync.h"
#include "cygerrno.h"
#include "sigproc.h"
#include "thread.h"
#include "cygtls.h"

#undef CloseHandle

static cygthread NO_COPY threads[32];
#define NTHREADS (sizeof (threads) / sizeof (threads[0]))

DWORD NO_COPY cygthread::main_thread_id;
bool NO_COPY cygthread::exiting;

/* Initial stub called by cygthread constructor. Performs initial
   per-thread initialization and loops waiting for new thread functions
   to execute.  */
DWORD WINAPI
cygthread::stub (VOID *arg)
{
  cygthread *info = (cygthread *) arg;
  _my_tls._ctinfo = info;
  if (info->arg == cygself)
    {
      if (info->ev)
	{
	  CloseHandle (info->ev);
	  CloseHandle (info->thread_sync);
	}
      info->ev = info->thread_sync = info->stack_ptr = NULL;
    }
  else
    {
      info->stack_ptr = &arg;
      debug_printf ("thread '%s', id %p, stack_ptr %p", info->name (), info->id, info->stack_ptr);
      if (!info->ev)
	{
	  info->ev = CreateEvent (&sec_none_nih, TRUE, FALSE, NULL);
	  info->thread_sync = CreateEvent (&sec_none_nih, FALSE, FALSE, NULL);
	}
    }

  while (1)
    {
      if (!info->__name)
#ifdef DEBUGGING
	system_printf ("erroneous thread activation, name is NULL prev thread name = '%s'", info->__oldname);
#else
	system_printf ("erroneous thread activation, name is NULL");
#endif
      else
	{
	  if (exiting)
	    {
	      info->inuse = false;	// FIXME: Do we need this?
	      return 0;
	    }

	  /* Cygwin threads should not call ExitThread directly */
	  info->func (info->arg == cygself ? info : info->arg);
	  /* ...so the above should always return */

	  /* If stack_ptr is NULL, the above function has set that to indicate
	     that it doesn't want to alert anyone with a SetEvent and should
	     just be marked as no longer inuse.  Hopefully the function knows
	     that it is doing.  */
	  if (!info->func)
	    info->release (false);
	  else
	    {
#ifdef DEBUGGING
	      info->func = NULL;	// catch erroneous activation
	      info->__oldname = info->__name;
#endif
	      info->__name = NULL;
	      SetEvent (info->ev);
	    }
	}
      switch (WaitForSingleObject (info->thread_sync, INFINITE))
	{
	case WAIT_OBJECT_0:
	  continue;
	default:
	  api_fatal ("WFSO failed, %E");
	  break;
	}
    }
}

/* Overflow stub called by cygthread constructor. Calls specified function
   and then exits the thread.  */
DWORD WINAPI
cygthread::simplestub (VOID *arg)
{
  cygthread *info = (cygthread *) arg;
  _my_tls._ctinfo = info;
  info->stack_ptr = &arg;
  info->ev = info->h;
  info->func (info->arg == cygself ? info : info->arg);
  return 0;
}

/* Start things going.  Called from dll_crt0_1. */
void
cygthread::init ()
{
  main_thread_id = GetCurrentThreadId ();
}

cygthread *
cygthread::freerange ()
{
  cygthread *self = (cygthread *) calloc (1, sizeof (*self));
  self->is_freerange = true;
  self->inuse = 1;
  return self;
}

void * cygthread::operator
new (size_t)
{
  cygthread *info;

  /* Search the threads array for an empty slot to use */
  for (info = threads; info < threads + NTHREADS; info++)
    if (!InterlockedExchange (&info->inuse, 1))
      {
	/* available */
#ifdef DEBUGGING
	if (info->__name)
	  api_fatal ("name not NULL? id %p, i %d", info->id, info - threads);
#endif
	goto out;
      }

#ifdef DEBUGGING
  char buf[1024];
  if (!GetEnvironmentVariable ("CYGWIN_FREERANGE_NOCHECK", buf, sizeof (buf)))
    api_fatal ("Overflowed cygwin thread pool");
  else
    thread_printf ("Overflowed cygwin thread pool");
#endif

  info = freerange ();	/* exhausted thread pool */

out:
  return info;
}

cygthread::cygthread (LPTHREAD_START_ROUTINE start, LPVOID param,
		      const char *name): __name (name),
					 func (start), arg (param)
{
  thread_printf ("name %s, id %p", name, id);
  if (h)
    {
      while (!thread_sync)
	low_priority_sleep (0);
      SetEvent (thread_sync);
      thread_printf ("activated name '%s', thread_sync %p for thread %p", name, thread_sync, id);
    }
  else
    {
      stack_ptr = NULL;
#ifdef DEBUGGING
      if (__oldname)
	system_printf ("__oldname %s, terminated %d", __oldname, terminated);
#endif
      h = CreateThread (&sec_none_nih, 0, is_freerange ? simplestub : stub,
			this, 0, &id);
      if (!h)
	api_fatal ("thread handle not set - %p<%p>, %E", h, id);
      thread_printf ("created name '%s', thread %p, id %p", name, h, id);
#ifdef DEBUGGING
      terminated = false;
#endif
    }
}

/* Return the symbolic name of the current thread for debugging.
 */
const char *
cygthread::name (DWORD tid)
{
  const char *res = NULL;
  if (!tid)
    tid = GetCurrentThreadId ();

  if (tid == main_thread_id)
    return "main";

  for (DWORD i = 0; i < NTHREADS; i++)
    if (threads[i].id == tid)
      {
	res = threads[i].__name ?: "exiting thread";
	break;
      }

  if (!res)
    {
      static char buf[30] NO_COPY = {0};
      __small_sprintf (buf, "unknown (%p)", tid);
      res = buf;
    }

  return res;
}

cygthread::operator
HANDLE ()
{
  while (!ev)
    low_priority_sleep (0);
  return ev;
}

void
cygthread::release (bool nuke_h)
{
  if (nuke_h)
    h = NULL;
#ifdef DEBUGGING
  __oldname = __name;
  debug_printf ("released thread '%s'", __oldname);
#endif
  __name = NULL;
  func = NULL;
  if (!InterlockedExchange (&inuse, 0))
#ifdef DEBUGGING
    api_fatal ("released a thread that was not inuse");
#else
    system_printf ("released a thread that was not inuse");
#endif
}

/* Forcibly terminate a thread. */
void
cygthread::terminate_thread ()
{
  /* FIXME: The if (!inuse) stuff below should be handled better.  The
     problem is that terminate_thread could be called while a thread
     is terminating and either the thread could be handling its own
     release or, if this is being called during exit, some other
     thread may be attempting to free up this resource.  In the former
     case, setting some kind of "I deal with my own exit" type of
     flag may be the way to handle this. */
  if (!is_freerange)
    {
      ResetEvent (*this);
      ResetEvent (thread_sync);
    }

  debug_printf ("thread '%s', id %p, inuse %d, stack_ptr %p", name (), id, inuse, stack_ptr);
  while (inuse && !stack_ptr)
    low_priority_sleep (0);

  if (!inuse)
    return;

  (void) TerminateThread (h, 0);
  (void) WaitForSingleObject (h, INFINITE);
  if (!inuse || exiting)
    return;

  CloseHandle (h);

  if (!inuse)
    return;

  MEMORY_BASIC_INFORMATION m;
  memset (&m, 0, sizeof (m));
  (void) VirtualQuery (stack_ptr, &m, sizeof m);

  if (!m.RegionSize)
    system_printf ("m.RegionSize 0?  stack_ptr %p", stack_ptr);
  else if (!VirtualFree (m.AllocationBase, 0, MEM_RELEASE))
    debug_printf ("VirtualFree of allocation base %p<%p> failed, %E",
		   stack_ptr, m.AllocationBase);

  if (!inuse)
    /* nothing */;
  if (is_freerange)
    free (this);
  else
    {
#ifdef DEBUGGING
      terminated = true;
#endif
      release (true);
    }
}

/* Detach the cygthread from the current thread.  Note that the
   theory is that cygthreads are only associated with one thread.
   So, there should be never be multiple threads doing waits
   on the same cygthread. */
bool
cygthread::detach (HANDLE sigwait)
{
  bool signalled = false;
  if (!inuse)
    system_printf ("called detach but inuse %d, thread %p?", inuse, id);
  else
    {
      DWORD res;

      if (!sigwait)
	res = WaitForSingleObject (*this, INFINITE);
      else
	{
	  HANDLE w4[2];
	  w4[0] = *this;
	  w4[1] = signal_arrived;
	  res = WaitForSingleObject (sigwait, INFINITE);
	  if (res != WAIT_OBJECT_0)
	    system_printf ("WFSO sigwait %p failed, res %u, %E", sigwait, res);
	  res = WaitForMultipleObjects (2, w4, FALSE, INFINITE);
	  if (res == WAIT_OBJECT_0)
	    /* nothing */;
	  else if (WaitForSingleObject (sigwait, 0) == WAIT_OBJECT_0)
	    res = WaitForSingleObject (*this, INFINITE);
	  else if ((res = WaitForSingleObject (*this, 0)) != WAIT_OBJECT_0)
	    {
	      signalled = true;
	      terminate_thread ();
	      set_sig_errno (EINTR);	/* caller should be dealing with return
					   values. */
	    }
	}

      thread_printf ("%s returns %d, id %p", sigwait ? "WFMO" : "WFSO",
		     res, id);

      if (signalled)
	/* already handled */;
      else if (is_freerange)
	{
	  CloseHandle (h);
	  free (this);
	}
      else
	{
	  ResetEvent (*this);
	  /* Mark the thread as available by setting inuse to zero */
	  (void) InterlockedExchange (&inuse, 0);
	}
    }
  return signalled;
}

void
cygthread::terminate ()
{
  exiting = 1;
}
