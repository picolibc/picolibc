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

static cygthread NO_COPY threads[6];
#define NTHREADS (sizeof (threads) / sizeof (threads[0]))

DWORD NO_COPY cygthread::main_thread_id;
int NO_COPY cygthread::initialized;

/* Initial stub called by cygthread constructor. Performs initial
   per-thread initialization and loops waiting for new thread functions
   to execute.  */
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
  info->ev = CreateEvent (&sec_none_nih, TRUE, FALSE, NULL);
  while (1)
    {
      if (!info->func)
	ExitThread (0);

      /* Cygwin threads should not call ExitThread directly */
      info->func (info->arg == cygself ? info : info->arg);
      /* ...so the above should always return */

#ifdef DEBUGGING
      info->func = NULL;	// catch erroneous activation
#endif
      SetEvent (info->ev);
      info->__name = NULL;
      if (initialized < 0)
	ExitThread (0);
      else
	SuspendThread (info->h);
    }
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
      return 0;

  initialized ^= 1;
  return 0;
}

/* Start things going.  Called from dll_crt0_1. */
void
cygthread::init ()
{
  DWORD tid;
  HANDLE h = CreateThread (&sec_none_nih, 0, cygthread::runner, NULL, 0, &tid);
  if (!h)
    api_fatal ("can't start thread_runner, %E");
  CloseHandle (h);
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
	  if (GetEnvironmentVariable ("CYGWIN_NOFREERANGE", buf, sizeof (buf)))
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
  while (!h || ResumeThread (h) != 1)
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
   theory is that cygthread's are only associated with one thread.
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
  initialized = -1;
  for (cygthread *info = threads + NTHREADS - 1; info >= threads; info--)
    if (!(DWORD) InterlockedExchange ((LPLONG) &info->avail, 0) && info->id)
      SetEvent (info->ev);
}
