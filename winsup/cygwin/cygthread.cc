/* cygthread.cc

   Copyright 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <windows.h>
#include <stdlib.h>
#include "exceptions.h"
#include "security.h"
#include "cygthread.h"

#undef CloseHandle

static cygthread NO_COPY threads[9];
#define NTHREADS (sizeof (threads) / sizeof (threads[0]))

DWORD NO_COPY cygthread::main_thread_id;
int NO_COPY cygthread::initialized;

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
    info->ev = info->thread_sync = NULL;
  else
    {
      info->ev = CreateEvent (&sec_none_nih, TRUE, FALSE, NULL);
      info->thread_sync = CreateEvent (&sec_none_nih, FALSE, FALSE, NULL);
    }
  while (1)
    {
      if (!info->__name)
	system_printf ("erroneous thread activation");
      else
	{
	  if (!info->func || initialized < 0)
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
	  // ResetEvent (info->thread_sync);
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

/* This function runs in a secondary thread and starts up a bunch of
   other suspended threads for use in the cygthread pool. */
DWORD WINAPI
cygthread::runner (VOID *arg)
{
  for (unsigned i = 0; i < NTHREADS; i++)
    if (!initialized)
      threads[i].h = CreateThread (&sec_none_nih, 0, cygthread::stub,
				   &threads[i], CREATE_SUSPENDED,
				   &threads[i].avail);
    else
      ExitThread (0);

  initialized ^= 1;
  ExitThread (0);
}

HANDLE NO_COPY runner_handle;
DWORD NO_COPY runner_tid;
/* Start things going.  Called from dll_crt0_1. */
void
cygthread::init ()
{
  runner_handle = CreateThread (&sec_none_nih, 0, cygthread::runner, NULL, 0,
				&runner_tid);
  if (!runner_handle)
    api_fatal ("can't start thread_runner, %E");
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

void *
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

  for (;;)
    {
      int was_initialized = initialized;
      if (was_initialized < 0)
	ExitThread (0);

      /* Search the threads array for an empty slot to use */
      for (info = threads + NTHREADS - 1; info >= threads; info--)
	if ((id = (DWORD) InterlockedExchange ((LPLONG) &info->avail, 0)))
	  {
	    info->id = id;
#ifdef DEBUGGING
	    if (info->__name)
	      api_fatal ("name not NULL? id %p, i %d", id, info - threads);
#endif
	    return info;
	  }

      if (was_initialized < 0)
	ExitThread (0);

      if (!was_initialized)
	Sleep (0); /* thread_runner is not finished yet. */
      else
	{
#ifdef DEBUGGING
	  char buf[1024];
	  if (!GetEnvironmentVariable ("CYGWIN_NOFREERANGE_NOCHECK", buf, sizeof (buf)))
	    api_fatal ("Overflowed cygwin thread pool");
#endif
	  return freerange ();
	}
    }
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
    Sleep (0);
#else
    {
      thread_printf ("waiting for %s<%p> to become active", __name, h);
      Sleep (0);
    }
#endif
  __name = name;	/* Need to set after thread has woken up to
			   ensure that it won't be cleared by exiting
			   thread. */
  if (thread_sync)
    SetEvent (thread_sync);
  else
    ResumeThread (h);
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
    Sleep (0);
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

/* Detach the cygthread from the current thread.  Note that the
   theory is that cygthreads are only associated with one thread.
   So, there should be no problems with multiple threads doing waits
   on the one cygthread. */
void
cygthread::detach ()
{
  if (avail)
    system_printf ("called detach on available thread %d?", avail);
  else
    {
      DWORD avail = id;
      /* Checking for __name here is just a minor optimization to avoid
	 an OS call. */
      if (!__name)
	thread_printf ("thread id %p returned.  No need to wait.", id);
      else
	{
	  DWORD res = WaitForSingleObject (*this, INFINITE);
	  thread_printf ("WFSO returns %d, id %p", res, id);
	}
      if (is_freerange)
	{
	  CloseHandle (h);
	  free (this);
	}
      else
	{
	  ResetEvent (*this);
	  id = 0;
	  __name = NULL;
	  /* Mark the thread as available by setting avail to non-zero */
	  (void) InterlockedExchange ((LPLONG) &this->avail, avail);
	}
    }
}

void
cygthread::terminate ()
{
  /* Wow.  All of this seems to be necessary or (on Windows 9x at least) the
     process will sometimes deadlock if there are suspended threads.  I assume
     that something funky is happening like a suspended thread being created
     while the process is exiting or something.  In particular, it seems like
     the WaitForSingleObjects are necessary since it appears that the
     TerminateThread call may happen asynchronously, i.e., when TerminateThread
     returns, the thread may not yet have terminated. */
  if (runner_handle && initialized >= 0)
    {
      /* Don't care about detaching (or attaching) threads now */
      if (cygwin_hmodule && !DisableThreadLibraryCalls (cygwin_hmodule))
	system_printf ("DisableThreadLibraryCalls (%p) failed, %E",
		       cygwin_hmodule);
      initialized = -1;
      (void) TerminateThread (runner_handle, 0);
      (void) WaitForSingleObject (runner_handle, INFINITE);
      (void) CloseHandle (runner_handle);
      HANDLE hthreads[NTHREADS];
      int n = 0;
      for (unsigned i = 0; i < NTHREADS; i++)
	if (threads[i].h)
	  {
	    hthreads[n] = threads[i].h;
	    threads[i].h = NULL;
	    TerminateThread (hthreads[n++], 0);
	  }
      if (n)
	{
	  (void) WaitForMultipleObjects (n, hthreads, TRUE, INFINITE);
	  while (--n >= 0)
	    CloseHandle (hthreads[n]);
	}
    }
}
