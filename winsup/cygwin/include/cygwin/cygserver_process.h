/* cygserver_process.h

   Copyright 2001 Red Hat Inc.

   Written by Robert Collins <rbtcollins@hotmail.com>

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#ifndef _CYGSERVER_PROCESS_
#define _CYGSERVER_PROCESS_

/* needs threaded_queue.h */

class process_cleanup:public queue_request
{
public:
  virtual void process ();
  process_cleanup (class process *nprocess) : theprocess (nprocess) {};
private:
  class process * theprocess;
};

class process_process_param:public queue_process_param
{
  class process_cache *cache;
public:
  DWORD request_loop ();
  process_process_param ():queue_process_param (true) {};
};

class cleanup_routine
{
public:
  cleanup_routine () : next (NULL) {};
  class cleanup_routine * next;
  /* MUST BE SYNCHRONOUS */
  virtual void cleanup (long winpid);
};

class process
{
public:
  HANDLE handle ();
  long winpid;
  process (long);
  ~process ();
  DWORD exit_code ();
  class process * next;
  long refcount;
  bool add_cleanup_routine (class cleanup_routine *);
  void cleanup ();
private:
  /* used to prevent races-on-delete */
  CRITICAL_SECTION access;
  volatile long cleaning_up;
  class cleanup_routine *head;
  HANDLE thehandle;
  DWORD _exit_status;
};

class process_cache:public threaded_queue
{
public:
  process_cache (unsigned int initial_workers);
  virtual ~ process_cache ();
  class process *process (long);
  /* remove a process from the cache */
  int handle_snapshot (HANDLE *, class process **, ssize_t, int);
  void remove_process (class process *);
  /* threaded_queue methods */
  void process_requests ();
  HANDLE cache_add_trigger;

private:
  virtual void add_task (class process *);
  class process *head;
  CRITICAL_SECTION cache_write_access;
};

#endif /* _CYGSERVER_PROCESS_ */
