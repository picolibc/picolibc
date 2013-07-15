/* threaded_queue.h

   Copyright 2001, 2002, 2003, 2012 Red Hat Inc.

   Written by Robert Collins <rbtcollins@hotmail.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _THREADED_QUEUE_
#define _THREADED_QUEUE_

/*****************************************************************************/

/* a specific request */

class queue_request
{
public:
  queue_request *_next;

  queue_request () : _next (NULL) {}
  virtual ~queue_request ();

  virtual void process () = 0;
};

/*****************************************************************************/

/* a queue to allocate requests from n submission loops to x worker threads */

class queue_submission_loop;

class threaded_queue
{
public:
  threaded_queue (size_t initial_workers = 1);
  ~threaded_queue ();

  void add_submission_loop (queue_submission_loop *);

  bool running () const { return _running; }

  bool start ();
  bool stop ();

  void add (queue_request *);

private:
  LONG _workers_count;
  bool _running;

  queue_submission_loop *_submitters_head;

  long _requests_count;		// Informational only.
  queue_request *_requests_head;

  CRITICAL_SECTION _queue_lock;
  HANDLE _requests_sem;		// == _requests_count

  static DWORD WINAPI start_routine (LPVOID /* this */);

  void create_workers (size_t initial_workers);
  void worker_loop ();
};

/*****************************************************************************/

/* parameters for a request finding and submitting loop */

class queue_submission_loop
{
  friend class threaded_queue;

public:
  queue_submission_loop (threaded_queue *, bool ninterruptible);
  virtual ~queue_submission_loop ();

  bool start ();
  bool stop ();

  threaded_queue *queue () { return _queue; };

protected:
  bool _running;
  HANDLE _interrupt_event;
  threaded_queue *const _queue;

private:
  bool _interruptible;
  HANDLE _hThread;
  DWORD _tid;
  queue_submission_loop *_next;

  static DWORD WINAPI start_routine (LPVOID /* this */);
  virtual void request_loop () = 0;
};

#ifdef __cplusplus

/*---------------------------------------------------------------------------*
 * Some type-safe versions of the various interlocked functions.
 *---------------------------------------------------------------------------*/

template <typename T> T *
TInterlockedExchangePointer (T **lvalue, T *rvalue)
{
  return reinterpret_cast<T *>
    (InterlockedExchangePointer (reinterpret_cast<void **> (lvalue),
				 reinterpret_cast<void *> (rvalue)));
}

template <typename T> T *
TInterlockedCompareExchangePointer (T **lvalue, T *rvalue1, T *rvalue2)
{
  return reinterpret_cast<T *>
    (InterlockedCompareExchangePointer (reinterpret_cast<void **> (lvalue),
					reinterpret_cast<void *> (rvalue1),
					reinterpret_cast<void *> (rvalue2)));
}

#endif /* __cplusplus */

#endif /* _THREADED_QUEUE_ */
