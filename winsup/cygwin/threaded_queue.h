/* threaded_queue.h

   Copyright 2001 Red Hat Inc.

   Written by Robert Collins <rbtcollins@hotmail.com>

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#ifndef _THREADED_QUEUE_
#define _THREADED_QUEUE_

/* a specific request */

class queue_request
{
  public:
    class queue_request *next;
    virtual void process ();
    queue_request();
};


typedef DWORD WINAPI threaded_queue_thread_function (LPVOID);
/* parameters for a request finding and submitting loop */

class queue_process_param
{
  public:
    bool start (threaded_queue_thread_function *, class threaded_queue *);
    void stop ();
    bool running;
    long int shutdown;
    class queue_process_param * next;
    class threaded_queue *queue;
    queue_process_param (bool ninterruptible);
    ~queue_process_param ();
    bool interruptible;
    HANDLE interrupt;
    HANDLE hThread;
    DWORD tid;
};

/* a queue to allocate requests from n submission loops to x worker threads */

class threaded_queue
{
  public:
    CRITICAL_SECTION queuelock;
    HANDLE event;
    bool active;
    queue_request * request;
    unsigned int initial_workers;
    unsigned int running;
    void create_workers ();
    void cleanup ();
    void add (queue_request *);
    void process_requests (queue_process_param *, threaded_queue_thread_function *);
  private:
    queue_request *process_head;
};

#endif /* _THREADED_QUEUE_ */
