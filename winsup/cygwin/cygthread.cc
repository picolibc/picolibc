/* cygthread.cc

   Copyright 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <windows.h>
#include <stdlib.h>
#include "exceptions.h"
#include "security.h"
#include "cygthread.h"
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
      if (!info->ev)
	{
	  info->ev = CreateEvent (&sec_none_nih, TRUE, FALSE, NULL);
	  info->thread_sync = CreateEvent (&sec_none_nih, FALSE, FALSE, NULL);
	}
    }

  while (1)
    {
      if (!info->__name)
	system_printf ("erroneous thread activation");
      else
	{
	  if (!info->func || exiting)
	    return 0;

	  /* Cygwin threads should not call ExitThread directly */
	  info->func (info->arg == cygself ? info : info->arg);
	  /* ...so the above should always return */

#ifdef DEBUGGING
	  info->func = NULL;	// catch erroneous activation
#endif
	  info->__name = NULL;
	  SetEvent (info->ev);
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

bool
cygthread::is ()
{
  DWORD tid = GetCurrentThreadId ();

  for (DWORD i = 0; i < NTHREADS; i++)
    if (threads[i].id == tid)
      return 1;

  return 0;
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
      thread_printf ("activated thread_sync %p", thread_sync);
    }
  else
    {
      stack_ptr = NULL;
      h = CreateThread (&sec_none_nih, 0, is_freerange ? simplestub : stub,
			this, 0, &id);
      if (!h)
	api_fatal ("thread handle not set - %p<%p>, %E", h, id);
      thread_printf ("created thread %p", h);
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

/* Should only be called when the process is exiting since it
   leaves an open thread slot. */
void
cygthread::exit_thread ()
{
  if (!is_freerange)
    SetEvent (*this);
  ExitThread (0);
}

/* Forcibly terminate a thread. */
void
cygthread::terminate_thread ()
{
  if (!is_freerange)
    {
      ResetEvent (*this);
      ResetEvent (thread_sync);
    }
  (void) TerminateThread (h, 0);
  (void) WaitForSingleObject (h, INFINITE);
  CloseHandle (h);

  while (!stack_ptr)
    low_priority_sleep (0);

  MEMORY_BASIC_INFORMATION m;
  memset (&m, 0, sizeof (m));
  (void) VirtualQuery (stack_ptr, &m, sizeof m);

  if (!m.RegionSize)
    system_printf ("m.RegionSize 0?  stack_ptr %p", stack_ptr);
  else if (!VirtualFree (m.AllocationBase, 0, MEM_RELEASE))
    debug_printf ("VirtualFree of allocation base %p<%p> failed, %E",
		   stack_ptr, m.AllocationBase);

  if (is_freerange)
    free (this);
  else
    {
      h = NULL;
      __name = NULL;
      stack_ptr = NULL;
      (void) InterlockedExchange (&inuse, 0); /* No longer in use */
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
