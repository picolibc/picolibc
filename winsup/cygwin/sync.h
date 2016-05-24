/* sync.h: Header file for cygwin synchronization primitives.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#pragma once

/* FIXME: Note that currently this class cannot be allocated via `new' since
   there are issues with malloc and fork. */
class muto
{
public:
  char *name;
private:
  LONG sync;	/* Used to serialize access to this class. */
  LONG waiters;	/* Number of threads waiting for lock. */
  HANDLE bruteforce; /* event handle used to control waiting for lock. */
public:
  LONG visits;	/* Count of number of times a thread has called acquire. */
  void *tls;	/* Tls of lock owner. */
  // class muto *next;

  /* The real constructor. */
  muto __reg2 *init (const char *);

#if 0	/* FIXME: See comment in sync.cc */
  ~muto ()
#endif
  int __reg2 acquire (DWORD ms = INFINITE); /* Acquire the lock. */
  int __reg2 release (_cygtls * = &_my_tls); /* Release the lock. */

  bool __reg1 acquired ();
  void upforgrabs () {tls = this;}  // just set to an invalid address
  void __reg1 grab ();
  operator int () const {return !!name;}
};

class lock_process
{
  bool skip_unlock;
  static muto locker;
public:
  static void init () {locker.init ("lock_process");}
  void dont_bother () {skip_unlock = true;}
  lock_process (bool exiting = false)
  {
    locker.acquire ();
    skip_unlock = exiting;
  }
  void release ()
  {
    locker.release ();
    skip_unlock = true;
  }
  ~lock_process ()
  {
    if (!skip_unlock)
      release ();
  }
  operator LONG () const {return locker.visits; }
  static void force_release (_cygtls *tid) {locker.release (tid);}
  friend class dtable;
  friend class fhandler_fifo;
};
