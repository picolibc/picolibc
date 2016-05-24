/* process.h

   Written by Robert Collins <rbtcollins@hotmail.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _PROCESS_H
#define _PROCESS_H

#include <assert.h>

#include "threaded_queue.h"

class process_cleanup : public queue_request
{
public:
  process_cleanup (class process *const theprocess)
    : _process (theprocess)
  {
    assert (_process);
  }

  virtual ~process_cleanup ();

  virtual void process ();

private:
  class process *const _process;
};

class process;

class cleanup_routine
{
  friend class process;

public:
  cleanup_routine (void *const key)
    : _key (key),
      _next (NULL)
  {}

  virtual ~cleanup_routine () = 0;

  bool operator== (const cleanup_routine &rhs) const
  {
    return _key == rhs._key;
  }

  void *key () const { return _key; }

  /* MUST BE SYNCHRONOUS */
  virtual void cleanup (class process *) = 0;

private:
  void *const _key;
  cleanup_routine *_next;
};

class process_cache;

#define hold()		_hold(__FILE__,__LINE__)
#define release()	_release(__FILE__,__LINE__)

class process
{
  friend class process_cache;
  friend class process_cleanup;

public:
  process (pid_t cygpid, DWORD winpid);
  ~process ();

  pid_t cygpid () const { return _cygpid; }
  DWORD winpid () const { return _winpid; }
  HANDLE handle () const { return _hProcess; }

  bool is_active () const { return _exit_status == STILL_ACTIVE; }

  void _hold (const char *file, int line) {
    _log (file, line, LOG_DEBUG, "Try hold(%lu)", _cygpid);
    EnterCriticalSection (&_access);
    _log (file, line, LOG_DEBUG, "holding (%lu)", _cygpid);
  }
  void _release (const char *file, int line) {
    _log (file, line, LOG_DEBUG, "leaving (%lu)", _cygpid);
    LeaveCriticalSection (&_access);
  }

  bool add (cleanup_routine *);
  bool remove (const cleanup_routine *);

private:
  const pid_t _cygpid;
  const DWORD _winpid;
  HANDLE _hProcess;
  LONG _cleaning_up;
  DWORD _exit_status;		// Set in the constructor and in exit_code ().
  cleanup_routine *_routines_head;
  /* used to prevent races-on-delete */
  CRITICAL_SECTION _access;
  class process *_next;

  DWORD check_exit_code ();
  void cleanup ();
};

class process_cache
{
  // Number of special (i.e., non-process) handles in _wait_array.
  // See wait_for_processes () and sync_wait_array () for details.
  enum {
    SPECIALS_COUNT = 2
  };

  class submission_loop : public queue_submission_loop
  {
  public:
    submission_loop (process_cache *const cache, threaded_queue *const queue)
      : queue_submission_loop (queue, true),
	_cache (cache)
    {
      assert (_cache);
    }

  private:
    process_cache *const _cache;

    virtual void request_loop ();
  };

  friend class submission_loop;

public:
  process_cache (const size_t max_procs, const unsigned int initial_workers);
  ~process_cache ();

  class process *process (pid_t cygpid, DWORD winpid);

  bool running () const { return _queue.running (); }

  bool start () { return _queue.start (); }
  bool stop () { return _queue.stop (); }

private:
  threaded_queue _queue;
  submission_loop _submitter;

  size_t _processes_count;
  size_t _max_process_count;
  class process *_processes_head; // A list sorted by winpid.

  // Access to the _wait_array and related fields is not thread-safe,
  // since they are used solely by wait_for_processes () and its callees.

  HANDLE _wait_array[5 * MAXIMUM_WAIT_OBJECTS];
  class process *_process_array[5 * MAXIMUM_WAIT_OBJECTS];

  HANDLE _cache_add_trigger;	// Actually both add and remove.
  CRITICAL_SECTION _cache_write_access; // Actually both read and write access.

  void wait_for_processes (HANDLE interrupt);
  size_t sync_wait_array (HANDLE interrupt);
  void check_and_remove_process (const size_t index);

  class process *find (DWORD winpid, class process **previous = NULL);
};

#endif /* _PROCESS_H */
