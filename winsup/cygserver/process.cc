/* process.cc

   Written by Robert Collins <rbtcollins@hotmail.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifdef __OUTSIDE_CYGWIN__
#include "woutsup.h"

#include <sys/types.h>

#include <assert.h>
#include <stdlib.h>

#include "process.h"

#include "cygserver_ipc.h"

/*****************************************************************************/

#define elements(ARRAY) (sizeof (ARRAY) / sizeof (*ARRAY))

/*****************************************************************************/

process_cleanup::~process_cleanup ()
{
  delete _process;
}

void
process_cleanup::process ()
{
  _process->cleanup ();
}

/*****************************************************************************/

process::process (const pid_t cygpid, const DWORD winpid)
  : _cygpid (cygpid),
    _winpid (winpid),
    _hProcess (NULL),
    _cleaning_up (false),
    _exit_status (STILL_ACTIVE),
    _routines_head (NULL),
    _next (NULL)
{
  _hProcess = OpenProcess (PROCESS_ALL_ACCESS, FALSE, winpid);
  if (!_hProcess)
    {
      system_printf ("unable to obtain handle for new cache process %d(%u)",
		     _cygpid, _winpid);
      _hProcess = INVALID_HANDLE_VALUE;
      _exit_status = 0;
    }
  else
    debug_printf ("got handle %p for new cache process %d(%u)",
		  _hProcess, _cygpid, _winpid);
  InitializeCriticalSection (&_access);
  debug ("initialized (%u)", _cygpid);
}

process::~process ()
{
  debug ("deleting (%u)", _cygpid);
  DeleteCriticalSection (&_access);
  CloseHandle (_hProcess);
}

/* No need to be thread-safe as this is only ever called by
 * process_cache::check_and_remove_process ().  If it has to be made
 * thread-safe later on, it should not use the `access' critical section as
 * that is held by the client request handlers for an arbitrary length of time,
 * i.e. while they do whatever processing is required for a client request.
 */
DWORD
process::check_exit_code ()
{
  if (_hProcess && _hProcess != INVALID_HANDLE_VALUE
      && _exit_status == STILL_ACTIVE
      && !GetExitCodeProcess (_hProcess, &_exit_status))
    {
      system_printf ("failed to retrieve exit code for %d(%u), error = %u",
		     _cygpid, _winpid, GetLastError ());
      _hProcess = INVALID_HANDLE_VALUE;
    }
  return _exit_status;
}

bool
process::add (cleanup_routine *const entry)
{
  assert (entry);

  bool res = false;
  hold ();

  if (!_cleaning_up)
    {
      entry->_next = _routines_head;
      _routines_head = entry;
      res = true;
    }

  release ();
  return res;
}

bool
process::remove (const cleanup_routine *const entry)
{
  assert (entry);

  bool res = false;
  hold ();

  if (!_cleaning_up)
    {
      cleanup_routine *previous = NULL;

      for (cleanup_routine *ptr = _routines_head;
	   ptr;
	   previous = ptr, ptr = ptr->_next)
	{
	  if (*ptr == *entry)
	    {
	      if (previous)
		previous->_next = ptr->_next;
	      else
		_routines_head = ptr->_next;

	      delete ptr;
	      res = true;
	      break;
	    }
	}
    }

  release ();
  return res;
}

/* This is single threaded. It's called after the process is removed
 * from the cache, but inserts may be attemped by worker threads that
 * have a pointer to it.
 */
void
process::cleanup ()
{
  hold ();
  assert (!is_active ());
  assert (!_cleaning_up);
  InterlockedExchange (&_cleaning_up, true);
  cleanup_routine *entry = _routines_head;
  _routines_head = NULL;
  release ();

  while (entry)
    {
      cleanup_routine *const ptr = entry;
      entry = entry->_next;
      ptr->cleanup (this);
      delete ptr;
    }
}

/*****************************************************************************/

void
process_cache::submission_loop::request_loop ()
{
  assert (_cache);
  assert (_interrupt_event);

  while (_running)
    _cache->wait_for_processes (_interrupt_event);
}

/*****************************************************************************/

process_cache::process_cache (const size_t max_procs,
			      const unsigned int initial_workers)
  : _queue (initial_workers),
    _submitter (this, &_queue),	// true == interruptible
    _processes_count (0),
    _max_process_count (max_procs),
    _processes_head (NULL),
    _cache_add_trigger (NULL)
{
  /* there can only be one */
  InitializeCriticalSection (&_cache_write_access);

  _cache_add_trigger = CreateEvent (NULL,  // SECURITY_ATTRIBUTES
				    TRUE,  // Manual-reset
				    FALSE, // Initially non-signalled
				    NULL); // Anonymous

  if (!_cache_add_trigger)
    {
      system_printf ("failed to create cache add trigger, error = %u",
		     GetLastError ());
      abort ();
    }

  _queue.add_submission_loop (&_submitter);
}

process_cache::~process_cache ()
{
  (void) CloseHandle (_cache_add_trigger);
  DeleteCriticalSection (&_cache_write_access);
}

/* This returns the process object to the caller already locked, that
 * is, with the object's `access' critical region entered.  Thus the
 * caller must unlock the object when it's finished with it (via
 * process::release ()).  It must then not try to access the object
 * afterwards, except by going through this routine again, as it may
 * have been deleted once it has been unlocked.
 */
class process *
process_cache::process (const pid_t cygpid, const DWORD winpid)
{
  /* TODO: make this more granular, so a search doesn't involve the
   * write lock.
   */
  EnterCriticalSection (&_cache_write_access);
  class process *previous = NULL;
  class process *entry = find (winpid, &previous);

  if (!entry)
    {
      if (_processes_count >= _max_process_count)
	{
	  LeaveCriticalSection (&_cache_write_access);
	  system_printf (("process limit (%d processes) reached; "
			  "new connection refused for %d(%u)"),
			 _max_process_count, cygpid, winpid);
	  return NULL;
	}

      entry = new class process (cygpid, winpid);
      if (!entry->is_active ())
	{
	  LeaveCriticalSection (&_cache_write_access);
	  delete entry;
	  return NULL;
	}

      if (previous)
	{
	  entry->_next = previous->_next;
	  previous->_next = entry;
	}
      else
	{
	  entry->_next = _processes_head;
	  _processes_head = entry;
	}

      _processes_count += 1;
      SetEvent (_cache_add_trigger);
    }

  entry->hold (); // To be released by the caller.
  LeaveCriticalSection (&_cache_write_access);
  assert (entry);
  assert (entry->_winpid == winpid);
  return entry;
}

struct pcache_wait_t
{
  size_t index;
  size_t count;
  HANDLE *hdls;
};

static DWORD WINAPI
pcache_wait_thread (const LPVOID param)
{
  pcache_wait_t *p = (pcache_wait_t *) param;

  DWORD rc = WaitForMultipleObjects (p->count, p->hdls, FALSE, INFINITE);
  ExitThread (rc == WAIT_FAILED ? rc : rc + p->index);
}

void
process_cache::wait_for_processes (const HANDLE interrupt_event)
{
  // Update `_wait_array' with handles of all current processes.
  size_t idx;
  const size_t count = sync_wait_array (interrupt_event);

  debug_printf ("waiting on %u objects in total (%u processes)",
		count, _processes_count);

  DWORD rc = WAIT_FAILED;

  if (count <= 64)
    {
      /* If count <= 64, a single WaitForMultipleObjects is sufficient and
         we can simply wait in the main thread. */
      rc = WaitForMultipleObjects (count, _wait_array, FALSE, INFINITE);
      if (rc == WAIT_FAILED)
	{
	  system_printf ("could not wait on the process handles, error = %u",
			 GetLastError ());
	  abort ();
	}
    }
  else
    {
      /* If count > 64 we have to create sub-threads which wait for the
         actual wait objects and the main thread waits for the termination
	 of one of the threads. */
      HANDLE main_wait_array[5] = { NULL };
      DWORD mcount = 0;

      for (idx = 0; idx < count; idx += 64)
	{
	  pcache_wait_t p = { idx, min (count - idx, 64), _wait_array + idx };
	  main_wait_array[mcount++] = CreateThread (NULL, 0, pcache_wait_thread,
						    &p, 0, NULL);
	}

      rc = WaitForMultipleObjects (mcount, main_wait_array, FALSE, INFINITE);
      if (rc == WAIT_FAILED)
	{
	  system_printf ("could not wait on the process handles, error = %u",
			 GetLastError ());
	  abort ();
	}

      /* Check for error condition on signalled sub-thread. */
      GetExitCodeThread (main_wait_array[rc], &rc);
      if (rc == WAIT_FAILED)
	{
	  system_printf ("could not wait on the process handles, error = %u",
			 GetLastError ());
	  abort ();
	}

      /* Wake up all waiting threads.  _cache_add_trigger gets reset
         in sync_wait_array again. */
      SetEvent (_cache_add_trigger);
      WaitForMultipleObjects (mcount, main_wait_array, TRUE, INFINITE);
      for (idx = 0; idx < mcount; idx++)
	CloseHandle (main_wait_array[idx]);
    }

  /* Tell all processes the bad news.  This one formerly only checked
     processes beginning with the index of the signalled process, but
     this can result in processes which are signalled but never removed
     under heavy load conditions. */
  for (idx = 0; idx < count; idx++)
    if (_process_array[idx])
      check_and_remove_process (idx);
}

/*
 * process_cache::sync_wait_array ()
 *
 * Fill-in the wait array with the handles that the cache needs to wait on.
 * These handles are:
 *  - the process_process_param's interrupt event
 *  - the process_cache's cache_add_trigger event
 *  - the handle for each live process in the cache.
 *
 * Return value: the number of live handles in the array.
 */

size_t
process_cache::sync_wait_array (const HANDLE interrupt_event)
{
  assert (interrupt_event && interrupt_event != INVALID_HANDLE_VALUE);

  /* Always reset _cache_add_trigger before filling up the array again. */
  ResetEvent (_cache_add_trigger);

  EnterCriticalSection (&_cache_write_access);

  size_t index = 0;

  for (class process *ptr = _processes_head; ptr; ptr = ptr->_next)
    {
      assert (ptr->_hProcess && ptr->_hProcess != INVALID_HANDLE_VALUE);
      assert (ptr->is_active ());

      _wait_array[index] = ptr->handle ();
      _process_array[index++] = ptr;

      if (!ptr->_next || index % 64 == 62)
	{
	  /* Added at the end of each thread's array part for efficiency. */
	  _wait_array[index] = interrupt_event;
	  _process_array[index++] = NULL;
	  _wait_array[index] = _cache_add_trigger;
	  _process_array[index++] = NULL;
	}
    }

  if (!index)
    {
      /* To get at least *something* to wait for. */
      _wait_array[index] = interrupt_event;
      _process_array[index++] = NULL;
      _wait_array[index] = _cache_add_trigger;
      _process_array[index++] = NULL;
    }

  assert (index <= elements (_wait_array));

  LeaveCriticalSection (&_cache_write_access);

  return index;
}

void
process_cache::check_and_remove_process (const size_t index)
{
  assert (index < elements (_wait_array) - SPECIALS_COUNT);

  class process *const process = _process_array[index];

  assert (process);
  assert (process->handle () == _wait_array[index]);

  if (process->check_exit_code () == STILL_ACTIVE)
    return;

  debug_printf ("process %d(%u) has left the building ($? = %u)",
		process->_cygpid, process->_winpid, process->_exit_status);

  /* Unlink the process object from the process list. */

  EnterCriticalSection (&_cache_write_access);

  class process *previous = NULL;

  const class process *const tmp = find (process->_winpid, &previous);

  assert (tmp == process);
  assert (previous ? previous->_next == process : _processes_head == process);

  if (previous)
    previous->_next = process->_next;
  else
    _processes_head = process->_next;

  _processes_count -= 1;
  LeaveCriticalSection (&_cache_write_access);

  /* Schedule any cleanup tasks for this process. */
  _queue.add (new process_cleanup (process));
}

class process *
process_cache::find (const DWORD winpid, class process **previous)
{
  if (previous)
    *previous = NULL;

  for (class process *ptr = _processes_head; ptr; ptr = ptr->_next)
    if (ptr->_winpid == winpid)
      return ptr;
    else if (ptr->_winpid > winpid) // The list is sorted by winpid.
      return NULL;
    else if (previous)
      *previous = ptr;

  return NULL;
}

void
thread::dup_signal_arrived ()
{
  if (ipcblk && ipcblk->signal_arrived
      && !DuplicateHandle (client->handle (), ipcblk->signal_arrived,
			   GetCurrentProcess (), &ipcblk->signal_arrived,
			   0, FALSE, DUPLICATE_SAME_ACCESS))
      {
	system_printf ("error duplicating thread's signal_arrived "
		       "to server (%u)", GetLastError ());
	ipcblk->signal_arrived = NULL;
      }
}

void
thread::close_signal_arrived ()
{
  if (ipcblk && ipcblk->signal_arrived)
    CloseHandle (ipcblk->signal_arrived);
}

/*****************************************************************************/
#endif /* __OUTSIDE_CYGWIN__ */
