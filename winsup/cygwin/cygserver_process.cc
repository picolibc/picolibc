/* cygserver_process.cc

   Copyright 2001 Red Hat Inc.

   Written by Robert Collins <rbtcollins@hotmail.com>

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <windows.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "wincap.h"
#include <pthread.h>
#include <threaded_queue.h>
#include <cygwin/cygserver_process.h>

#define debug_printf if (DEBUG) printf
#define DEBUG 1

/* the cache structures and classes are designed for one cache per server process.
 * To make multiple process caches, a redesign will be needed
 */

/* process cache */
process_cache::process_cache (unsigned int num_initial_workers):
head (NULL)
{
  /* there can only be one */
  InitializeCriticalSection (&cache_write_access);
  if ((cache_add_trigger = CreateEvent (NULL, FALSE, FALSE, NULL)) == NULL)
    {
      printf ("Failed to create cache add trigger (%lu), terminating\n",
	      GetLastError ());
      exit (1);
    }
  initial_workers = num_initial_workers;
}

process_cache::~process_cache ()
{
}

class process *
process_cache::process (long pid)
{
  class process *entry = head;
  /* TODO: make this more granular, so a search doesn't involve the write lock */
  EnterCriticalSection (&cache_write_access);
  if (!entry)
    {
      entry = new class process (pid);
      entry->next =
	(class process *) InterlockedExchangePointer (&head, entry);
      PulseEvent (cache_add_trigger);
    }
  else
    {
      while (entry->winpid != pid && entry->next)
	entry = entry->next;
      if (entry->winpid != pid)
	{
	  class process *new_entry = new class process (pid);
	  new_entry->next =
	    (class process *) InterlockedExchangePointer (&entry->next,
							  new_entry);
	  entry = new_entry;
	  PulseEvent (cache_add_trigger);
	}
    }
  LeaveCriticalSection (&cache_write_access);
  return entry;
}

static DWORD WINAPI
request_loop (LPVOID LpParam)
{
  class process_process_param *params = (process_process_param *) LpParam;
  return params->request_loop ();
}

void
process_cache::process_requests ()
{
  class process_process_param *params = new process_process_param;
  threaded_queue::process_requests (params, request_loop);
}

void
process_cache::add_task (class process * theprocess)
{
  /* safe to not "Try" because workers don't hog this, they wait on the event
   */
  /* every derived ::add must enter the section! */
  EnterCriticalSection (&queuelock);
  queue_request *listrequest = new process_cleanup (theprocess);
  threaded_queue::add (listrequest);
  LeaveCriticalSection (&queuelock);
}

/* NOT fully MT SAFE: must be called by only one thread in a program */
void
process_cache::remove_process (class process *theprocess)
{
  class process *entry = head;
  /* unlink */
  EnterCriticalSection (&cache_write_access);
  if (entry == theprocess)
    {
      entry = (class process *) InterlockedExchangePointer (&head, theprocess->next);
      if (entry != theprocess)
        {
          printf ("Bug encountered, process cache corrupted\n");
	  exit (1);
        }
    }
  else
    {
      while (entry->next && entry->next != theprocess)
	entry = entry->next;
      class process *temp = (class process *) InterlockedExchangePointer (&entry->next, theprocess->next);
      if (temp != theprocess)
	{
	  printf ("Bug encountered, process cache corrupted\n");
	  exit (1);
	}
    }
  LeaveCriticalSection (&cache_write_access);
  /* Process any cleanup tasks */
  add_task (theprocess);
}
	
      
/* copy <= max_copy HANDLEs to dest[], starting at an offset into _our list_ of
 * begin_at. (Ie begin_at = 5, the first copied handle is still written to dest[0]
 * NOTE: Thread safe, but not thread guaranteed - a newly added process may be missed. 
 * Who cares - It'll get caught the next time.
 */
int
process_cache::handle_snapshot (HANDLE * hdest, class process ** edest,
				ssize_t max_copy, int begin_at)
{
  /* TODO:? grab a delete-lock, to prevent deletes during this process ? */
  class process *entry = head;
  int count = begin_at;
  /* skip begin_at entries */
  while (entry && count)
    {
      if (entry->exit_code () == STILL_ACTIVE)
	count--;
      entry = entry->next;
    }
  /* hit the end of the list within begin_at entries */
  if (count)
    return 0;
  HANDLE *hto = hdest;
  class process **eto = edest;
  while (entry && count < max_copy)
    {
      /* hack */
      if (entry->exit_code () == STILL_ACTIVE)
	{
	  *hto = entry->handle ();
	  *eto = entry;
	  count++;
	  hto++;
	  eto++;
	}
      entry = entry->next;
    }
  return count;
}

/* process's */
/* global process crit section */
static CRITICAL_SECTION process_access;
static pthread_once_t process_init;

void
do_process_init (void)
{
  InitializeCriticalSection (&process_access);
  /* we don't have a cache shutdown capability today */
}

process::process (long pid):
winpid (pid), next (NULL), cleaning_up (0), head (NULL), _exit_status (STILL_ACTIVE)
{
  pthread_once (&process_init, do_process_init);
  EnterCriticalSection (&process_access);
  thehandle = OpenProcess (PROCESS_ALL_ACCESS, FALSE, pid);
  if (!thehandle)
    {
      printf ("unable to obtain handle for new cache process %ld\n", pid);
      thehandle = INVALID_HANDLE_VALUE;
    }
  debug_printf ("Got handle %p for new cache process %ld\n", thehandle, pid);
  InitializeCriticalSection (&access);
  LeaveCriticalSection (&process_access);
}

process::~process ()
{
  DeleteCriticalSection (&access);
}

HANDLE
process::handle ()
{
//  DWORD exitstate = exit_code ();
//  if (exitstate == STILL_ACTIVE)
  return thehandle;

  /* FIXME: call the cleanup list ? */

//  CloseHandle (thehandle);
//  debug_printf ("Process id %ld has terminated, attempting to open a new handle\n",
//       winpid);
//  thehandle = OpenProcess (PROCESS_ALL_ACCESS, FALSE, winpid);
//  debug_printf ("Got handle %p when refreshing cache process %ld\n", thehandle, winpid);
//  /* FIXME: what if OpenProcess fails ? */
//  if (thehandle) 
//    {
//      _exit_status = STILL_ACTIVE;
//      exit_code ();
//    }
//  else
//    thehandle = INVALID_HANDLE_VALUE;
//  return thehandle;
}

DWORD process::exit_code ()
{
  if (_exit_status != STILL_ACTIVE)
    return _exit_status;
  bool
    err = GetExitCodeProcess (thehandle, &_exit_status);
  if (!err)
    {
      debug_printf ("Failed to retrieve exit code (%ld)\n", GetLastError ());
      thehandle = INVALID_HANDLE_VALUE;
      return _exit_status;
    }
  else if (_exit_status == STILL_ACTIVE)
    return _exit_status;
  /* add new cleanup task etc etc ? */
  return _exit_status;
}

/* this is single threaded. It's called after the process is removed from the cache,
 * but inserts may be attemped by worker threads that have a pointer to it */
void
process::cleanup ()
{
  /* Serialize this */
  EnterCriticalSection (&access);
  InterlockedIncrement (&(long)cleaning_up);
  class cleanup_routine *entry = head;
  while (entry)
    {
      class cleanup_routine *temp;
      entry->cleanup (winpid);
      temp = entry->next;
      delete entry;
      entry = temp;
    }
  LeaveCriticalSection (&access);
}

bool
process::add_cleanup_routine (class cleanup_routine *new_cleanup)
{
  if (cleaning_up)
    return false;
  EnterCriticalSection (&access);
  /* check that we didn't block with ::cleanup () 
   * This rigmarole is to get around win9x's glaring missing TryEnterCriticalSection call
   * which would be a whole lot easier
   */
  if (cleaning_up)
    {
      LeaveCriticalSection (&access);
      return false;
    }
  new_cleanup->next = head;
  head = new_cleanup;
  LeaveCriticalSection (&access);
  return true;
}

/* process_cleanup */
void
process_cleanup::process ()
{
  theprocess->cleanup ();
  delete theprocess;
}

/* process_process_param */
DWORD
process_process_param::request_loop ()
{
  process_cache *cache = (process_cache *) queue;
  /* always malloc one, so there is no special case in the loop */
  ssize_t HandlesSize = 2;
  HANDLE *Handles = (HANDLE *) malloc (sizeof (HANDLE) * HandlesSize);
  process **Entries = (process **) malloc (sizeof (LPVOID) * HandlesSize);
  /* TODO: put [1] at the end as it will also get done if a process dies? */
  Handles[0] = interrupt;
  Handles[1] = cache->cache_add_trigger;
  while (cache->active && !shutdown)
    {
      int copied;
      copied = -1;
      int offset;
      offset = 1;
      int count;
      count = 2;
      while ((copied == HandlesSize - 2 - offset) || copied < 0)
	{
	  /* we need more storage to cope with all the HANDLES */
	  if (copied == HandlesSize - 2 - offset)
	    {
	      HANDLE *temp = (HANDLE *) realloc (Handles,
						 sizeof (HANDLE) *
						 HandlesSize + 10);
	      if (!temp)
		{
		  printf
		    ("cannot allocate more storage for the handle array!\n");
		  exit (1);
		}
	      Handles = temp;
	      process **ptemp = (process **) realloc (Entries,
						      sizeof (LPVOID) *
						      HandlesSize + 10);
	      if (!ptemp)
		{
		  printf
		    ("cannot allocate more storage for the handle array!\n");
		  exit (1);
		}
	      Entries = ptemp;
	      HandlesSize += 10;
	    }
	  offset += copied;
	  copied =
	    cache->handle_snapshot (&Handles[2], &Entries[2],
				    HandlesSize - 2 - offset, offset);
	  count += copied;
	}
      debug_printf ("waiting on %u objects\n", count);
      DWORD rc = WaitForMultipleObjects (count, Handles, FALSE, INFINITE);
      if (rc == WAIT_FAILED)
	{
	  printf ("Could not wait on the process handles (%ld)!\n",
		  GetLastError ());
	  exit (1);
	}
      int objindex = rc - WAIT_OBJECT_0;
      if (objindex > 1 && objindex < count)
	{
	  debug_printf ("Process %ld has left the building\n",
			Entries[objindex]->winpid);
	  /* fire off the termination routines */
	  cache->remove_process (Entries[objindex]);
	}
      else if (objindex >= 0 && objindex < 2)
	{
	  /* 0 is shutdown - do nothing */
	  /* 1 is a cache add event - just rebuild the object list */
	}
      else
	{
	  printf
	    ("unexpected return code from WaitForMultiple objects in process_process_param::request_loop\n");
	}
    }
  running = false;
  return 0;
}
