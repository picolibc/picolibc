/* cygtls.h

   Copyright 2003 Red Hat, Inc.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGTLS_H
#define _CYGTLS_H

#include <cygwin/signal.h>

#define CYGTLS_INITIALIZED 0x43227
#define CYGTLS_EXCEPTION (0x43227 + true)

/* Please keep this file simple.  Changes to the below structure may require
   acompanying changes to the very simple parser in the perl script
   'gentls_offsets' (<<-- start parsing here).  */

#pragma pack(push,4)
typedef __uint32_t __stack_t;
struct _threadinfo
{
  void (*func) /*gentls_offsets*/(int)/*gentls_offsets*/;
  int saved_errno;
  int sa_flags;
  sigset_t oldmask;
  sigset_t newmask;
  HANDLE event;
  int *errno_addr;
  unsigned initialized;
  sigset_t sigmask;
  sigset_t sigwait_mask;
  siginfo_t *sigwait_info;
  siginfo_t infodata;
  struct pthread *tid;
  struct _threadinfo *prev, *next;
  __stack_t stack[8];
  int sig;
  __stack_t *stackptr;

  /*gentls_offsets*/
  static CRITICAL_SECTION protect_linked_list;
  static void init ();
  void init_thread (void *);
  static void call (void (*) (void *, void *), void *);
  void call2 (void (*) (void *, void *), void *, void *);
  static struct _threadinfo *find_tls (int sig);
  void remove ();
  void push (__stack_t, bool = false);
  __stack_t pop ();
  bool isinitialized () {return initialized == CYGTLS_INITIALIZED || initialized == CYGTLS_EXCEPTION;}
  void set_state (bool);
  void reset_exception ();
  bool interrupt_now (CONTEXT *, int, void *, struct sigaction&)
    __attribute__((regparm(3)));
  void __stdcall interrupt_setup (int sig, void *handler, struct sigaction& siga, __stack_t retaddr)
    __attribute__((regparm(3)));
  operator HANDLE () const {return tid->win32_obj_id;}
  /*gentls_offsets*/
};
#pragma pack(pop)

extern _threadinfo *_tlsbase __asm__ ("%fs:4");
#define _my_tls (_tlsbase[-1])

#define CYGTLS_PADSIZE (sizeof (_threadinfo) + 64)
#endif /*_CYGTLS_H*/
