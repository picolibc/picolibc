/* sync.h: Header file for cygwin synchronization primitives.

   Copyright 1999, 2000, 2001, 2002, 2003, 2004 Red Hat, Inc.

   Written by Christopher Faylor <cgf@cygnus.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _SYNC_H
#define _SYNC_H
/* FIXME: Note that currently this class cannot be allocated via `new' since
   there are issues with malloc and fork. */
class muto
{
  static DWORD exiting_thread;
  static CRITICAL_SECTION init_lock;
  LONG sync;	/* Used to serialize access to this class. */
  LONG waiters;	/* Number of threads waiting for lock. */
  HANDLE bruteforce; /* event handle used to control waiting for lock. */
public:
  LONG visits;	/* Count of number of times a thread has called acquire. */
  void *tls;	/* Tls of lock owner. */
  // class muto *next;
  const char *name;

  /* The real constructor. */
  muto *init(const char *name) __attribute__ ((regparm (3)));

#if 0	/* FIXME: See comment in sync.cc */
  ~muto ()
#endif
  int acquire (DWORD ms = INFINITE) __attribute__ ((regparm (2))); /* Acquire the lock. */
  int release () __attribute__ ((regparm (1)));		     /* Release the lock. */

  bool acquired () __attribute__ ((regparm (1)));
  void upforgrabs () {tls = this;}  // just set to an invalid address
  void grab () __attribute__ ((regparm (1)));
  static void set_exiting_thread () {exiting_thread = GetCurrentThreadId ();}
  static void init ();
};

extern muto muto_start;

/* Use a statically allocated buffer as the storage for a muto */
#define new_muto(__name) \
({ \
  static muto __name##_storage __attribute__((nocommon)) __attribute__((section(".data_cygwin_nocopy1"))); \
  __name = __name##_storage.init (#__name); \
})

/* Use a statically allocated buffer as the storage for a muto */
#define new_muto1(__name, __storage) \
({ \
  static muto __storage __attribute__((nocommon)) __attribute__((section(".data_cygwin_nocopy1"))); \
  __name = __storage.init (#__name); \
})

/* Use a statically allocated buffer as the storage for a muto */
#define new_muto_name(__var, __name) \
({ \
  static muto __var##_storage __attribute__((nocommon)) __attribute__((section(".data_cygwin_nocopy1"))); \
  __var = __var##_storage.init (__name); \
})
#endif /*_SYNC_H*/
