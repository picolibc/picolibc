/* cygthread.cc

   Copyright 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "exceptions.h"
#include "security.h"
#include "cygthread.h"
#include <windows.h>

#undef CloseHandle

static cygthread NO_COPY threads[8];
#define NTHREADS (sizeof (threads) / sizeof (threads[0]))

static HANDLE NO_COPY hthreads[NTHREADS];

DWORD NO_COPY cygthread::main_thread_id;

/* Initial stub called by makethread. Performs initial per-thread
   initialization.  */
DWORD WINAPI
cygthread::stub (VOID *arg)
{
  DECLARE_TLS_STORAGE;
  exception_list except_entry;

  /* Initialize this thread's ability to respond to things like
     SIGSEGV or SIGFPE. */
  init_exceptions (&except_entry);

  cygthread *info = (cygthread *) arg;
  info->ev = CreateEvent (&sec_none_nih, FALSE, FALSE, NULL);
  while (1)
    {
      if (!info->func)
	ExitThread (0);

      /* Cygwin threads should not call ExitThread */
      info->func (info->arg);

      info->__name = NULL;
      SetEvent (info->ev);
      SuspendThread (info->h);
    }
}

DWORD WINAPI
cygthread::runner (VOID *arg)
{
  for (unsigned i = 0; i < NTHREADS; i++)
    hthreads[i] = threads[i].h =
      CreateThread (&sec_none_nih, 0, cygthread::stub, &threads[i],
		    CREATE_SUSPENDED, &threads[i].avail);
  return 0;
}

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
  cygthread *info; /* Various information needed by the newly created thread */

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

void
cygthread::detach ()
{
  if (!avail)
    {
      DWORD avail = id;
      if (__name)
	{
	  DWORD res = WaitForSingleObject (*this, INFINITE);
	  debug_printf ("WFSO returns %d", res);
	}
      id = 0;
      (void) InterlockedExchange ((LPLONG) &this->avail, avail);
    }
}
