/* perthread.h: Header file for cygwin thread-local storage.

   Copyright 2000, 2001, 2002, 2004 Red Hat, Inc.

   Written by Christopher Faylor <cgf@cygnus.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#define PTMAGIC 0x77366377

#define PER_THREAD_FORK_CLEAR ((void *)UINT32_MAX)
class per_thread
{
  DWORD tls;
  int clear_on_fork_p;
public:
  per_thread (int forkval = 1) {tls = TlsAlloc (); clear_on_fork_p = forkval;}
  DWORD get_tls () {return tls;}
  int clear_on_fork () {return clear_on_fork_p;}

  virtual void *get () {return TlsGetValue (get_tls ());}
  virtual size_t size () {return 0;}
  virtual void set (void *s = NULL);
  virtual void set (int n) {TlsSetValue (get_tls (), (void *)n);}
  virtual void *create ()
  {
    void *s = new char [size ()];
    memset (s, 0, size ());
    set (s);
    return s;
  }
};

class per_thread_waitq : public per_thread
{
public:
  per_thread_waitq () : per_thread (0) {}
  void *get () {return (waitq *) per_thread::get ();}
  void *create () {return (waitq *) per_thread::create ();}
  size_t size () {return sizeof (waitq);}
};

#ifdef NEED_VFORK
#include "cygtls.h"
#endif

#ifndef NEWVFORK
#define VFORKPID 0
#else
#if defined (NEED_VFORK)
class vfork_save
{
  jmp_buf j;
  int exitval;
 public:
  int pid;
  DWORD frame[100];
  _threadinfo tls;
  char **vfork_ebp;
  char **vfork_esp;
  int ctty;
  pid_t sid;
  pid_t pgid;
  int open_fhs;
  int is_active () { return pid < 0; }
  void restore_pid (int val)
  {
    pid = val;
    longjmp (j, 1);
  }
  void restore_exit (int val)
  {
    exitval = val;
    longjmp (j, 1);
  }
  friend int vfork ();
};

class per_thread_vfork : public per_thread
{
public:
  vfork_save *val () { return (vfork_save *) per_thread::get (); }
  vfork_save *create () {return (vfork_save *) per_thread::create ();}
  size_t size () {return sizeof (vfork_save);}
};
extern per_thread_vfork vfork_storage;
extern vfork_save *main_vfork;
#define VFORKPID main_vfork->pid
#endif
#endif /*NEWVFORK*/

extern per_thread_waitq waitq_storage;

extern per_thread *threadstuff[];
