/* threaded_queue.cc

   Copyright 2001 Red Hat Inc.

   Written by Robert Collins <rbtcollins@hotmail.com>

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <windows.h>
#include <sys/types.h>
#include "wincap.h"
#include "threaded_queue.h"
#define DEBUG 1
#define debug_printf if (DEBUG) printf

/* threaded_queue */

DWORD WINAPI
worker_function (LPVOID LpParam)
{
  class threaded_queue *queue = (class threaded_queue *) LpParam;
  class queue_request *request;
  /* FIXME use a threadsafe pop instead for speed? */
  while (queue->active)
    {
      EnterCriticalSection (&queue->queuelock);
      while (!queue->request && queue->active)
	{
	  LeaveCriticalSection (&queue->queuelock);
	  DWORD rc = WaitForSingleObject (queue->event, INFINITE);
	  if (rc == WAIT_FAILED)
	    {
	      printf ("Wait for event failed\n");
	      queue->running--;
	      ExitThread (0);
	    }
	  EnterCriticalSection (&queue->queuelock);
	}
      if (!queue->active)
	{
	  queue->running--;
	  LeaveCriticalSection (&queue->queuelock);
	  ExitThread (0);
	}
      /* not needed, but it is efficient */
      request =
	(class queue_request *) InterlockedExchangePointer (&queue->request,
							    queue->request->
							    next);
      LeaveCriticalSection (&queue->queuelock);
      request->process ();
      delete request;
    }
  queue->running--;
  ExitThread (0);
}

void
threaded_queue::create_workers ()
{
  InitializeCriticalSection (&queuelock);
  if ((event = CreateEvent (NULL, FALSE, FALSE, NULL)) == NULL)
    {
      printf ("Failed to create event queue (%lu), terminating\n",
	      GetLastError ());
      exit (1);
    }
  active = true;

  /* FIXME: Use a stack pair and create threads on the fly whenever
   * we have to to service a request.
   */
  for (unsigned int i = 0; i < initial_workers; i++)
    {
      HANDLE hThread;
      DWORD tid;
      hThread = CreateThread (NULL, 0, worker_function, this, 0, &tid);
      if (hThread == NULL)
	{
	  printf ("Failed to create thread (%lu), terminating\n",
		  GetLastError ());
	  exit (1);
	}
      CloseHandle (hThread);
      running++;
    }
}

void
threaded_queue::cleanup ()
{
  /* harvest the threads */
  active = false;
  /* kill the request processing loops */
  queue_process_param *reqloop;
  /* make sure we don't race with a incoming request creation */
  EnterCriticalSection (&queuelock);
  reqloop =
    (queue_process_param *) InterlockedExchangePointer (&process_head, NULL);
  while (reqloop)
    {
      queue_process_param *t = reqloop;
      reqloop = reqloop->next;
      delete t;
    }
  LeaveCriticalSection (&queuelock);
  if (!running)
    return;
  printf ("Waiting for current connections to terminate\n");
  for (int n = running; n; n--)
    PulseEvent (event);
  while (running)
    sleep (1);
  DeleteCriticalSection (&queuelock);
  CloseHandle (event);
}

/* FIXME: return success or failure */
void
threaded_queue::add (queue_request * therequest)
{
  /* safe to not "Try" because workers don't hog this, they wait on the event
   */
  EnterCriticalSection (&queuelock);
  if (!running)
    {
      printf ("No worker threads to handle request!\n");
    }
  if (!request)
    request = therequest;
  else
    {
      /* add to the queue end. */
      queue_request *listrequest = request;
      while (listrequest->next)
	listrequest = listrequest->next;
      listrequest->next = therequest;
    }
  PulseEvent (event);
  LeaveCriticalSection (&queuelock);
}

/* FIXME: return success or failure rather than quitting */
void
threaded_queue::process_requests (queue_process_param * params,
				  threaded_queue_thread_function *
				  request_loop)
{
  if (params->start (request_loop, this) == false)
    exit (1);
  params->next =
    (queue_process_param *) InterlockedExchangePointer (&process_head,
							params);
}

/* queue_process_param */
/* How does a constructor return an error? */
queue_process_param::queue_process_param (bool ninterruptible):running (false), shutdown (false),
interruptible
(ninterruptible)
{
  if (!interruptible)
    return;
  debug_printf ("creating an interruptible processing thread\n");
  if ((interrupt = CreateEvent (NULL, FALSE, FALSE, NULL)) == NULL)
    {
      printf ("Failed to create interrupt event (%lu), terminating\n",
	      GetLastError ());
      exit (1);
    }
}

queue_process_param::~queue_process_param ()
{
  debug_printf ("aaaargh\n");
  if (running)
    stop ();
  if (!interruptible)
    return;
  CloseHandle (interrupt);
}

bool
  queue_process_param::start (threaded_queue_thread_function * request_loop,
			      threaded_queue * thequeue)
{
  queue = thequeue;
  hThread = CreateThread (NULL, 0, request_loop, this, 0, &tid);
  if (hThread)
    {
      running = true;
      return true;
    }
  printf ("Failed to create thread (%lu), terminating\n", GetLastError ());
  return false;
}

void
queue_process_param::stop ()
{
  if (interruptible)
    {
      InterlockedExchange (&shutdown, true);
      PulseEvent (interrupt);
      /* Wait up to 50 ms for the thread to exit. If it doesn't _and_ we get
       * scheduled again, we print an error and exit. We _should_ loop or
       * try resignalling. We don't want to hand here though...
       */
      if (WaitForSingleObject (hThread, 200) == WAIT_TIMEOUT)
	{
	  printf ("Process thread didn't shutdown cleanly after 200ms!\n");
	  exit (1);
	}
      else
	running = false;
    }
  else
    {
      printf ("killing request loop thread %ld\n", tid);
      int rc;
      if (!(rc = TerminateThread (hThread, 0)))
	{
	  printf ("error shutting down request loop worker thread\n");
	}
      running = false;
    }
  CloseHandle (hThread);
}

/* queue_request */
queue_request::queue_request ():next (NULL)
{
}

void
queue_request::process (void)
{
  printf ("\n**********************************************\n"
	  "Oh no! we've hit the base queue_request process() function, and this indicates a coding\n"
	  "fault !!!\n" "***********************************************\n");
}
