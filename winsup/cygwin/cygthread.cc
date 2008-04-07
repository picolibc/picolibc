/* cygthread.cc

   Copyright 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005 Red Hat, Inc.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "miscfuncs.h"
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

void
cygthread::callfunc (bool issimplestub)
{
  void *pass_arg;
  if (arg == cygself)
    pass_arg = this;
  else if (!arglen)
    pass_arg = arg;
  else
    {
      if (issimplestub)
	ev = CreateEvent (&sec_none_nih, TRUE, FALSE, NULL);
      pass_arg = alloca (arglen);
      memcpy (pass_arg, arg, arglen);
      SetEvent (ev);
    }
  if (issimplestub)
    {
      /* Wait for main thread to assign 'h' */
      while (!h)
	low_priority_sleep (0);
      if (ev)
	CloseHandle (ev);
      ev = h;
    }
  /* Cygwin threads should not call ExitThread directly */
  func (pass_arg);
  /* ...so the above should always return */
}

/* Initial stub called by cygthread constructor. Performs initial
   per-thread initialization and loops waiting for another thread function
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

	  info->callfunc (false);

	  HANDLE notify = info->notify_detached;
	  /* If func is NULL, the above function has set that to indicate
	     that it doesn't want to alert anyone with a SetEvent and should
	     just be marked as no longer inuse.  Hopefully the function knows
	     what it is doing.  */
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
	  if (notify)
	    SetEvent (notify);
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
  info->callfunc (true);
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
	  api_fatal ("name not NULL? %s, id %p, i %d", info->__name, info->id, info - threads);
#endif
	goto out;
      }

#ifdef DEBUGGING
  if (!getenv ("CYGWIN_FREERANGE_NOCHECK"))
    api_fatal ("overflowed cygwin thread pool");
  else
    thread_printf ("overflowed cygwin thread pool");
#endif

  info = freerange ();	/* exhausted thread pool */

out:
  return info;
}

cygthread::cygthread (LPTHREAD_START_ROUTINE start, size_t n, void *param,
		      const char *name, HANDLE notify)
 : __name (name), func (start), arglen (n), arg (param), notify_detached (notify)
{
  thread_printf ("name %s, id %p", name, id);
  HANDLE htobe;
  if (h)
    {
      if (ev)
	ResetEvent (ev);
      while (!thread_sync)
	low_priority_sleep (0);
      SetEvent (thread_sync);
      thread_printf ("activated name '%s', thread_sync %p for thread %p", name, thread_sync, id);
      htobe = h;
    }
  else
    {
      stack_ptr = NULL;
      htobe = CreateThread (&sec_none_nih, 0, is_freerange ? simplestub : stub,
			    this, 0, &id);
      if (!htobe)
	api_fatal ("CreateThread failed for %s - %p<%p>, %E", name, h, id);
      thread_printf ("created name '%s', thread %p, id %p", name, h, id);
#ifdef DEBUGGING
      terminated = false;
#endif
    }

  if (n)
    {
      while (!ev)
	low_priority_sleep (0);
      WaitForSingleObject (ev, INFINITE);
      ResetEvent (ev);
    }
  h = htobe;
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
      __small_sprintf (_my_tls.locals.unknown_thread_name, "unknown (%p)", tid);
      res = _my_tls.locals.unknown_thread_name;
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
  /* Must be last */
  if (!InterlockedExchange (&inuse, 0))
#ifdef DEBUGGING
    api_fatal ("released a thread that was not inuse");
#else
    system_printf ("released a thread that was not inuse");
#endif
}

/* Forcibly terminate a thread. */
bool
cygthread::terminate_thread ()
{
  bool terminated = true;
  debug_printf ("thread '%s', id %p, inuse %d, stack_ptr %p", __name, id, inuse, stack_ptr);
  while (inuse && !stack_ptr)
    low_priority_sleep (0);

  if (!inuse)
    goto force_notterminated;

  TerminateThread (h, 0);
  WaitForSingleObject (h, INFINITE);
  CloseHandle (h);

  if (!inuse || exiting)
    goto force_notterminated;

  if (ev && !(terminated = WaitForSingleObject (ev, 0) != WAIT_OBJECT_0))
    ResetEvent (ev);

  MEMORY_BASIC_INFORMATION m;
  memset (&m, 0, sizeof (m));
  VirtualQuery (stack_ptr, &m, sizeof m);

  if (!m.RegionSize)
    system_printf ("m.RegionSize 0?  stack_ptr %p", stack_ptr);
  else if (!VirtualFree (m.AllocationBase, 0, MEM_RELEASE))
    debug_printf ("VirtualFree of allocation base %p<%p> failed, %E",
		   stack_ptr, m.AllocationBase);

  if (is_freerange)
    free (this);
  else
    {
#ifdef DEBUGGING
      terminated = true;
#endif
      release (true);
    }

  goto out;

force_notterminated:
  terminated = false;
out:
  return terminated;
}

/* Detach the cygthread from the current thread.  Note that the
   theory is that cygthreads are only associated with one thread.
   So, there should be never be multiple threads doing waits
   on the same cygthread. */
bool
cygthread::detach (HANDLE sigwait)
{
  bool signalled = false;
  bool thread_was_reset = false;
  if (!inuse)
    system_printf ("called detach but inuse %d, thread %p?", inuse, id);
  else
    {
      DWORD res;

      if (!sigwait)
	/* If the caller specified a special handle for notification, wait for that.
	   This assumes that the thread in question is auto releasing. */
	res = WaitForSingleObject (*this, INFINITE);
      else
	{
	  /* Lower our priority and give priority to the read thread */
	  HANDLE hth = GetCurrentThread ();
	  LONG prio = GetThreadPriority (hth);
	  ::SetThreadPriority (hth, THREAD_PRIORITY_BELOW_NORMAL);

	  HANDLE w4[2];
	  unsigned n = 2;
	  DWORD howlong = INFINITE;
	  w4[0] = sigwait;
	  w4[1] = signal_arrived;
	  /* For a description of the below loop see the end of this file */
	  for (int i = 0; i < 2; i++)
	    switch (res = WaitForMultipleObjects (n, w4, FALSE, howlong))
	      {
	      case WAIT_OBJECT_0:
		if (n == 1)
		  howlong = 50;
		break;
	      case WAIT_OBJECT_0 + 1:
		n = 1;
		if (i--)
		  howlong = 50;
		break;
	      case WAIT_TIMEOUT:
		break;
	      default:
		if (!exiting)
		  {
		    system_printf ("WFMO failed waiting for cygthread '%s', %E", __name);
		    for (unsigned j = 0; j < n; j++)
		      switch (WaitForSingleObject (w4[j], 0))
			{
			case WAIT_OBJECT_0:
			case WAIT_TIMEOUT:
			  break;
			default:
			  system_printf ("%s handle %p is bad", (j ? "signal_arrived" : "semaphore"), w4[j]);
			  break;
			}
		    api_fatal ("exiting on fatal error");
		  }
		break;
	      }
	  /* WAIT_OBJECT_0 means that the thread successfully read something,
	     so wait for the cygthread to "terminate". */
	  if (res == WAIT_OBJECT_0)
	    WaitForSingleObject (*this, INFINITE);
	  else
	    {
	      /* Thread didn't terminate on its own, so maybe we have to
		 do it. */
	      signalled = terminate_thread ();
	      /* Possibly the thread completed *just* before it was
		 terminated.  Detect this. If this happened then the
		 read was not terminated on a signal. */
	      if (WaitForSingleObject (sigwait, 0) == WAIT_OBJECT_0)
		signalled = false;
	      if (signalled)
		set_sig_errno (EINTR);
	      thread_was_reset = true;
	    }
	  ::SetThreadPriority (hth, prio);
	}

      thread_printf ("%s returns %d, id %p", sigwait ? "WFMO" : "WFSO",
		     res, id);

      if (thread_was_reset)
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
	  InterlockedExchange (&inuse, 0);
	}
    }
  return signalled;
}

void
cygthread::terminate ()
{
  exiting = 1;
}

/* The below is an explanation of synchronization loop in cygthread::detach.
   The intent is that the loop will always try hard to wait for both
   synchronization events from the reader thread but will exit with
   res == WAIT_TIMEOUT if a signal occurred and the reader thread is
   still blocked.

    case 0 - no signal

    i == 0 (howlong == INFINITE)
	W0 activated
	howlong not set because n != 1
	just loop

    i == 1 (howlong == INFINITE)
	W0 activated
	howlong not set because n != 1
	just loop (to exit loop) - no signal

    i == 2 (howlong == INFINITE)
	exit loop

    case 1 - signal before thread initialized

    i == 0 (howlong == INFINITE)
	WO + 1 activated
	n set to 1
	howlong untouched because i-- == 0
	loop

    i == 0 (howlong == INFINITE)
	W0 must be activated
	howlong set to 50 because n == 1

    i == 1 (howlong == 50)
	W0 activated
	loop (to exit loop) - no signal

	WAIT_TIMEOUT activated
	signal potentially detected
	loop (to exit loop)

    i == 2 (howlong == 50)
	exit loop

    case 2 - signal after thread initialized

    i == 0 (howlong == INFINITE)
	W0 activated
	howlong not set because n != 1
	loop

    i == 1 (howlong == INFINITE)
	W0 + 1 activated
	n set to 1
	howlong set to 50 because i-- != 0
	loop

    i == 1 (howlong == 50)
	W0 activated
	loop (to exit loop) - no signal

	WAIT_TIMEOUT activated
	loop (to exit loop) - signal

    i == 2 (howlong == 50)
	exit loop
*/
