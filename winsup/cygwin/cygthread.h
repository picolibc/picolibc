/* cygthread.h

   Copyright 1998, 1999, 2000, 2001, 2002, 2003, 2004 Red Hat, Inc.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGTHREAD_H
#define _CYGTHREAD_H

class cygthread
{
  LONG inuse;
  DWORD id;
  HANDLE h;
  HANDLE ev;
  HANDLE thread_sync;
  void *stack_ptr;
  const char *__name;
#ifdef DEBUGGING
  const char *__oldname;
  bool terminated;
#endif
  LPTHREAD_START_ROUTINE func;
  VOID *arg;
  bool is_freerange;
  static bool exiting;
 public:
  void terminate_thread ();
  static DWORD WINAPI stub (VOID *);
  static DWORD WINAPI simplestub (VOID *);
  static DWORD main_thread_id;
  static const char * name (DWORD = 0);
  void auto_release () {func = NULL;}
  void release (bool);
  cygthread (LPTHREAD_START_ROUTINE, LPVOID, const char *);
  cygthread () {};
  static void init ();
  bool detach (HANDLE = NULL);
  operator HANDLE ();
  void * operator new (size_t);
  static cygthread *freerange ();
  void exit_thread ();
  static void terminate ();
  bool SetThreadPriority (int nPriority) {return ::SetThreadPriority (h, nPriority);}
  void zap_h ()
  {
    (void) CloseHandle (h);
    h = NULL;
  }
};

#define cygself NULL
#endif /*_CYGTHREAD_H*/
