/* perthread.h: Header file for cygwin synchronization primitives.

   Copyright 2000 Cygnus Solutions.

   Written by Christopher Faylor <cgf@cygnus.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#define PTMAGIC 0x77366377

struct _reent;
extern struct _reent reent_data;

extern DWORD *__stackbase __asm__ ("%fs:4");

extern __inline struct _reent *
get_reent ()
{
  DWORD *base = __stackbase - 1;

  if (*base != PTMAGIC)
    return &reent_data;
  return (struct _reent *) base[-1];
}

extern inline void
set_reent (struct _reent *r)
{
  DWORD *base = __stackbase - 1;

  *base = PTMAGIC;
  base[-1] = (DWORD) r;
}

#define PER_THREAD_FORK_CLEAR ((void *)0xffffffff)
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
  void *get () {return (waitq *) this->per_thread::get ();}
  void *create () {return (waitq *) this->per_thread::create ();}
  size_t size () {return sizeof (waitq);}
};

#ifdef NEED_VFORK
struct vfork_save
{
  int pid;
  jmp_buf j;
  DWORD frame[100];
  char **vfork_ebp;
  char **vfork_esp;
  int is_active () { return pid < 0; }
};

class per_thread_vfork : public per_thread
{
public:
  vfork_save *val () { return (vfork_save *) this->per_thread::get (); }
  vfork_save *create () {return (vfork_save *) this->per_thread::create ();}
  size_t size () {return sizeof (vfork_save);}
};
extern per_thread_vfork vfork_storage;
#endif

extern "C" {
struct signal_dispatch
{
  int arg;
  void (*func) (int);
  int sig;
  int saved_errno;
  DWORD oldmask;
  DWORD newmask;
  DWORD retaddr;
  DWORD *retaddr_on_stack;
};
};

struct per_thread_signal_dispatch : public per_thread
{
  signal_dispatch *get () { return (signal_dispatch *) this->per_thread::get (); }
  signal_dispatch *create () {return (signal_dispatch *) this->per_thread::create ();}
  size_t size () {return sizeof (signal_dispatch);}
};

extern per_thread_waitq waitq_storage;
extern per_thread_signal_dispatch signal_dispatch_storage;

extern per_thread *threadstuff[];
