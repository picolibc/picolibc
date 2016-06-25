/* cygthread.h

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGTHREAD_H
#define _CYGTHREAD_H

typedef void WINAPI (*LPVOID_THREAD_START_ROUTINE) (void *) __attribute__((noreturn));		// Input queue thread

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
  unsigned arglen;
  VOID *arg;
  bool is_freerange;
  static bool exiting;
  HANDLE notify_detached;
  void __reg1 create ();
  static void CALLBACK async_create (ULONG_PTR);
 public:
  bool terminate_thread ();
  static DWORD WINAPI stub (VOID *);
  static DWORD WINAPI simplestub (VOID *);
  static DWORD main_thread_id;
  static const char *name (DWORD = 0);
  void __reg2 callfunc (bool) __attribute__ ((noinline, ));
  void auto_release () {func = NULL;}
  void release (bool);
  cygthread (LPTHREAD_START_ROUTINE start, unsigned n, LPVOID param, const char *name, HANDLE notify = NULL)
  : __name (name), func (start), arglen (n), arg (param),
  notify_detached (notify)
  {
    create ();
  }
  cygthread (LPVOID_THREAD_START_ROUTINE start, LPVOID param, const char *name)
  : __name (name), func ((LPTHREAD_START_ROUTINE) start), arglen (0),
    arg (param), notify_detached (NULL)
  {
    QueueUserAPC (async_create, GetCurrentThread (), (ULONG_PTR) this);
  }
  cygthread (LPTHREAD_START_ROUTINE start, LPVOID param, const char *name, HANDLE notify = NULL)
  : __name (name), func (start), arglen (0), arg (param),
  notify_detached (notify)
  {
    create ();
  }
  cygthread (LPVOID_THREAD_START_ROUTINE start, unsigned n, LPVOID param, const char *name)
  : __name (name), func ((LPTHREAD_START_ROUTINE) start), arglen (n),
    arg (param), notify_detached (NULL)
  {
    QueueUserAPC (async_create, GetCurrentThread (), (ULONG_PTR) this);
  }
  cygthread () {};
  static void init ();
  bool detach (HANDLE = NULL);
  operator HANDLE ();
  void * operator new (size_t);
  static cygthread *freerange ();
  static void terminate ();
  HANDLE thread_handle () const {return h;}
  bool SetThreadPriority (int nPriority) {return ::SetThreadPriority (h, nPriority);}
  void zap_h ()
  {
    CloseHandle (h);
    h = NULL;
  }
};

#define cygself NULL
#endif /*_CYGTHREAD_H*/
