/* cygthread.h

   Copyright 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

class cygthread
{
  DWORD avail;
  DWORD id;
  HANDLE h;
  HANDLE ev;
  const char *__name;
  LPTHREAD_START_ROUTINE func;
  VOID *arg;
  static DWORD main_thread_id;
  static DWORD WINAPI runner (VOID *);
  static DWORD WINAPI stub (VOID *);
 public:
  static const char * name (DWORD = 0);
  cygthread (LPTHREAD_START_ROUTINE, LPVOID, const char *);
  cygthread () {};
  static void init ();
  void detach ();
  operator HANDLE ();
  static bool is ();
  void * operator new (size_t);
  void exit_thread ();
};
