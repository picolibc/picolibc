/* cygthread.cc

   Copyright 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <windows.h>
#include <stdlib.h>
#include <errno.h>
#include "exceptions.h"
#include "security.h"
#include "cygthread.h"
#include "sync.h"
#include "cygerrno.h"
#include "sigproc.h"

#undef CloseHandle

static cygthread NO_COPY threads[18];
#define NTHREADS (sizeof (threads) / sizeof (threads[0]))

DWORD NO_COPY cygthread::main_thread_id;
bool NO_COPY cygthread::exiting;

/* Initial stub called by cygthread constructor. Performs initial
   per-thread initialization and loops waiting for new thread functions
   to execute.  */
DWORD WINAPI
cygthread::stub (VOID *arg)
{
  DECLARE_TLS_STORAGE;
  exception_list except_entry;

  /* Initialize this thread's ability to respond to things like
     SIGSEGV or SIGFPE. */
  init_exceptions (&except_entry);

  cygthread *info = (cygthread *) arg;
  if (info->arg == cygself)
    info->ev = info->thread_sync = info->stack_ptr = NULL;
  else
    {
      info->ev = CreateEvent (&sec_none_nih, TRUE, FALSE, NULL);
      info->thread_sync = CreateEvent (&sec_none_nih, FALSE, FALSE, NULL);
      info->stack_ptr = &arg;
    }
  while (1)
    {
      if (!info->__name)
	system_printf ("erroneous thread activation");
      else
	{
	  if (!info->func || exiting)
	    ExitThread (0);

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
  DECLARE_TLS_STORAGE;
  exception_list except_entry;

  /* Initialize this thread's ability to respond to things like
     SIGSEGV or SIGFPE. */
  init_exceptions (&except_entry);

  cygthread *info = (cygthread *) arg;
  info->func (info->arg == cygself ? info : info->arg);
  ExitThread (0);
}

static NO_COPY muto *cygthread_protect;
/* Start things going.  Called from dll_crt0_1. */
void
cygthread::init ()
{
  new_muto (cygthread_protect);
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
  self->h = CreateThread (&sec_none_nih, 0, cygthread::simplestub, self,
			  CREATE_SUSPENDED, &self->id);
  self->ev = self->h;
  return self;
}

void * cygthread::operator
new (size_t)
{
  DWORD id;
  cygthread *info;

  cygthread_protect->acquire ();

  /* Search the threads array for an empty slot to use */
  for (info = threads; info < threads + NTHREADS; info++)
    if ((id = (DWORD) InterlockedExchange ((LPLONG) &info->avail, 0)))
      {
#ifdef DEBUGGING
	if (info->__name)
	  api_fatal ("name not NULL? id %p, i %d", id, info - threads);
#endif
	goto out;
      }
    else if (!info->id)
      {
	info->h = CreateThread (&sec_none_nih, 0, cygthread::stub, info,
				CREATE_SUSPENDED, &info->id);
	goto out;
      }

#ifdef DEBUGGING
  char buf[1024];
  if (!GetEnvironmentVariable ("CYGWIN_NOFREERANGE_NOCHECK", buf, sizeof (buf)))
    api_fatal ("Overflowed cygwin thread pool");
#endif

  info = freerange ();	/* exhausted thread pool */

out:
  cygthread_protect->release ();
  return info;
}

cygthread::cygthread (LPTHREAD_START_ROUTINE start, LPVOID param,
		      const char *name): func (start), arg (param)
{
#ifdef DEBUGGGING
  if (!__name)
    api_fatal ("name should never be NULL");
#endif
  thread_printf ("name %s, id %p", name, id);
  while (!h)
#ifndef DEBUGGING
    low_priority_sleep (0);
#else
    {
      thread_printf ("waiting for %s<%p> to become active", __name, h);
      low_priority_sleep (0);
    }
#endif
  __name = name;
  if (!thread_sync)
    ResumeThread (h);
  else
    SetEvent (thread_sync);
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
    SetEvent (*this);
  (void) TerminateThread (h, 0);
  (void) WaitForSingleObject (h, INFINITE);

  MEMORY_BASIC_INFORMATION m;
  memset (&m, 0, sizeof (m));
  (void) VirtualQuery (stack_ptr, &m, sizeof m);

  if (m.RegionSize)
    (void) VirtualFree (m.AllocationBase, m.RegionSize, MEM_DECOMMIT);

  if (is_freerange)
    is_freerange = false;
  else
    {
      CloseHandle (ev);
      CloseHandle (thread_sync);
    }
  CloseHandle (h);
  thread_sync = ev = h = NULL;
  __name = NULL;
  id = 0;
}

/* Detach the cygthread from the current thread.  Note that the
   theory is that cygthreads are only associated with one thread.
   So, there should be no problems with multiple threads doing waits
   on the one cygthread. */
void
cygthread::detach (bool wait_for_sig)
{
  if (avail)
    system_printf ("called detach on available thread %d?", avail);
  else
    {
      DWORD avail = id;
      DWORD res;

      if (!wait_for_sig)
	res = WaitForSingleObject (*this, INFINITE);
      else
	{
	  HANDLE w4[2];
	  w4[0] = signal_arrived;
	  w4[1] = *this;
	  res = WaitForMultipleObjects (2, w4, FALSE, INFINITE);
	  if (res == WAIT_OBJECT_0)
	    {
	      terminate_thread ();
	      set_errno (EINTR);	/* caller should be dealing with return
					   values. */
	      avail = 0;
	    }
	}

      thread_printf ("%s returns %d, id %p", wait_for_sig ? "WFMO" : "WFSO",
		     res, id);

      if (!avail)
	/* already handled */;
      else if (is_freerange)
	{
	  CloseHandle (h);
	  free (this);
	}
      else
	{
	  ResetEvent (*this);
	  /* Mark the thread as available by setting avail to non-zero */
	  (void) InterlockedExchange ((LPLONG) &this->avail, avail);
	}
    }
}

void
cygthread::terminate ()
{
  exiting = 1;
}
