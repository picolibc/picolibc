/* cygthread.cc

   Copyright 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <windows.h>
#include "exceptions.h"
#include "security.h"
#include "cygthread.h"

#undef CloseHandle

static cygthread NO_COPY threads[8];
#define NTHREADS (sizeof (threads) / sizeof (threads[0]))

static HANDLE NO_COPY hthreads[NTHREADS];

DWORD NO_COPY cygthread::main_thread_id;

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

      /* Cygwin threads should not call ExitThread */
      info->func (info->arg);

      debug_printf ("returned from function %p", info->func);
      SetEvent (info->ev);
      info->__name = NULL;
      SuspendThread (info->h);
    }
}

/* This function runs in a secondary thread and starts up a bunch of
   other suspended threads for use in the cygthread pool. */
DWORD WINAPI
cygthread::runner (VOID *arg)
{
  for (unsigned i = 0; i < NTHREADS; i++)
    hthreads[i] = threads[i].h =
      CreateThread (&sec_none_nih, 0, cygthread::stub, &threads[i],
		    CREATE_SUSPENDED, &threads[i].avail);
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

void * cygthread::operator
new (size_t)
{
  DWORD id;
  cygthread *info;

  for (;;)
    {
      /* Search the threads array for an empty slot to use */
      for (info = threads; info < threads + NTHREADS; info++)
	if ((id = (DWORD) InterlockedExchange ((LPLONG) &info->avail, 0)))
	  {
	    info->id = id;
	    return info;
	  }

      /* thread_runner may not be finished yet. */
      Sleep (0);
    }
}

cygthread::cygthread (LPTHREAD_START_ROUTINE start, LPVOID param,
		      const char *name): __name (name), func (start), arg (param)
{
  while (ResumeThread (h) == 0)
    Sleep (0);
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
  SetEvent (ev);
  ExitThread (0);
}

/* Detach the cygthread from the current thread.  Note that the
   theory is that cygthread's are only associated with one thread.
   So, there should be no problems with multiple threads doing waits
   on the one cygthread. */
void
cygthread::detach ()
{
  if (!avail)
    {
      DWORD avail = id;
      /* Checking for __name here is just a minor optimization to avoid
	 an OS call. */
      if (!__name)
	debug_printf ("thread routine returned.  No need to wait.");
      else
	{
	  DWORD res = WaitForSingleObject (*this, INFINITE);
	  debug_printf ("WFSO returns %d", res);
	}
      ResetEvent (*this);
      id = 0;
      /* Mark the thread as available by setting avail to non-zero */
      (void) InterlockedExchange ((LPLONG) &this->avail, avail);
    }
}
