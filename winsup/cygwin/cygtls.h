/* cygtls.h

   Copyright 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013,
   2014 Red Hat, Inc.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#pragma once

#include <signal.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#define _NOMNTENT_FUNCS
#include <mntent.h>
#undef _NOMNTENT_FUNCS
#include <setjmp.h>

#define CYGTLS_INITIALIZED 0xc763173f

#ifndef CYG_MAX_PATH
# define CYG_MAX_PATH 260
#endif

#ifndef UNLEN
# define UNLEN 256
#endif

#define TLS_STACK_SIZE 256

#include "cygthread.h"

#define TP_NUM_C_BUFS 50
#define TP_NUM_W_BUFS 50

#ifdef CYGTLS_HANDLE
#include "thread.h"
#endif

#ifdef __x86_64__
#pragma pack(push,8)
#else
#pragma pack(push,4)
#endif
/* Defined here to support auto rebuild of tlsoffsets.h. */
class tls_pathbuf
{
  int c_cnt;
  int w_cnt;
  char  *c_buf[TP_NUM_C_BUFS];
  WCHAR *w_buf[TP_NUM_W_BUFS];

public:
  void destroy ();
  friend class tmp_pathbuf;
  friend class _cygtls;
  friend class san;
};

class unionent
{
public:
  char *name;
  char **list;
  short port_proto_addrtype;
  short h_len;
  union
  {
    char *s_proto;
    char **h_addr_list;
  };
  enum struct_type
  {
    t_hostent, t_protoent, t_servent
  };
};

struct _local_storage
{
  /* passwd.cc */
  char pass[_PASSWORD_LEN];

  /* dlfcn.cc */
  int dl_error;
  char dl_buffer[256];

  /* path.cc */
  struct mntent mntbuf;
  int iteration;
  unsigned available_drives;
  char mnt_type[80];
  char mnt_opts[80];
  char mnt_fsname[CYG_MAX_PATH];
  char mnt_dir[CYG_MAX_PATH];

  /* select.cc */
  struct {
    HANDLE  sockevt;
    int     max_w4;
    LONG   *ser_num;			// note: malloced
    HANDLE *w4;				// note: malloced
  } select;

  /* strerror errno.cc */
  char strerror_buf[sizeof ("Unknown error -2147483648")];
  char strerror_r_buf[sizeof ("Unknown error -2147483648")];

  /* times.cc */
  char timezone_buf[20];

  /* strsig.cc */
  char signamebuf[sizeof ("Unknown signal 4294967295   ")];

  /* net.cc */
  char *ntoa_buf;			// note: malloced
  unionent *hostent_buf;		// note: malloced
  unionent *protoent_buf;		// note: malloced
  unionent *servent_buf;		// note: malloced

  /* cygthread.cc */
  char unknown_thread_name[30];

  /* syscalls.cc */
  int setmode_file;
  int setmode_mode;

  /* thread.cc */
  HANDLE cw_timer;

  /* All functions requiring temporary path buffers. */
  tls_pathbuf pathbufs;
  char ttybuf[32];
};

typedef struct struct_waitq
{
  int pid;
  int options;
  int status;
  HANDLE ev;
  void *rusage;			/* pointer to potential rusage */
  struct struct_waitq *next;
  HANDLE thread_ev;
} waitq;

/* Changes to the below structure may require acompanying changes to the very
   simple parser in the perl script 'gentls_offsets' (<<-- start parsing here).
   The union in this structure is used to force alignment between the version
   of the compiler used to generate tlsoffsets.h and the cygwin cross compiler.
*/

/*gentls_offsets*/

extern "C" int __sjfault (jmp_buf);
extern "C" int __ljfault (jmp_buf, int);

/*gentls_offsets*/

typedef uintptr_t __stack_t;

class _cygtls
{
public:
  /* Please keep these two declarations first */
  struct _local_storage locals;
  union
  {
    struct _reent local_clib;
    char __dontuse[8 * ((sizeof(struct _reent) + 4) / 8)];
  };
  /**/
  void (*func) /*gentls_offsets*/(int, siginfo_t *, void *)/*gentls_offsets*/;
  int saved_errno;
  int sa_flags;
  sigset_t oldmask;
  sigset_t deltamask;
  int *errno_addr;
  sigset_t sigmask;
  sigset_t sigwait_mask;
  siginfo_t *sigwait_info;
  HANDLE signal_arrived;
  bool will_wait_for_signal;
  struct ucontext thread_context;
  DWORD thread_id;
  siginfo_t infodata;
  struct pthread *tid;
  class cygthread *_ctinfo;
  class san *andreas;
  waitq wq;
  int sig;
  unsigned incyg;
  unsigned spinning;
  unsigned stacklock;
  __stack_t *stackptr;
  __stack_t stack[TLS_STACK_SIZE];
  unsigned initialized;

  /*gentls_offsets*/
  void init_thread (void *, DWORD (*) (void *, void *));
  static void call (DWORD (*) (void *, void *), void *);
  void remove (DWORD);
  void push (__stack_t addr) {*stackptr++ = (__stack_t) addr;}
  __stack_t __reg1 pop ();
  __stack_t retaddr () {return stackptr[-1];}
  bool isinitialized () const
  {
    return initialized == CYGTLS_INITIALIZED;
  }
  bool __reg3 interrupt_now (CONTEXT *, siginfo_t&, void *, struct sigaction&);
  void __reg3 interrupt_setup (siginfo_t&, void *, struct sigaction&);

  bool inside_kernel (CONTEXT *);
  void __reg2 signal_debugger (siginfo_t&);

#ifdef CYGTLS_HANDLE
  operator HANDLE () const {return tid ? tid->win32_obj_id : NULL;}
#endif
  int __reg1 call_signal_handler ();
  void __reg1 remove_wq (DWORD);
  void __reg1 fixup_after_fork ();
  void __reg1 lock ();
  void __reg1 unlock ();
  bool __reg1 locked ();
  HANDLE get_signal_arrived (bool wait_for_lock = true)
  {
    if (!signal_arrived)
      {
	if (wait_for_lock)
	  lock ();
	if (!signal_arrived)
	  signal_arrived = CreateEvent (NULL, false, false, NULL);
	if (wait_for_lock)
	  unlock ();
      }
    return signal_arrived;
  }
  void set_signal_arrived (bool setit, HANDLE& h)
  {
    if (!setit)
      will_wait_for_signal = false;
    else
      {
	h = get_signal_arrived ();
	will_wait_for_signal = true;
      }
  }
  void reset_signal_arrived ()
  {
    if (signal_arrived)
      ResetEvent (signal_arrived);
    will_wait_for_signal = false;
  }
  void handle_SIGCONT ();
private:
  void __reg3 call2 (DWORD (*) (void *, void *), void *, void *);
  /*gentls_offsets*/
};
#pragma pack(pop)

/* FIXME: Find some way to autogenerate this value */
#ifdef __x86_64__
const int CYGTLS_PADSIZE = 12800;	/* Must be 16-byte aligned */
#else
const int CYGTLS_PADSIZE = 12700;
#endif

/*gentls_offsets*/

#include "cygerrno.h"
#include "ntdll.h"

#ifdef __x86_64__
/* When just using a "gs:X" asm for the x86_64 code, gcc wrongly creates
   pc-relative instructions.  However, NtCurrentTeb() is inline assembler
   anyway, so using it here should be fast enough on x86_64. */
#define _tlsbase (NtCurrentTeb()->Tib.StackBase)
#define _tlstop (NtCurrentTeb()->Tib.StackLimit)
#else
extern PVOID _tlsbase __asm__ ("%fs:4");
extern PVOID _tlstop __asm__ ("%fs:8");
#endif
#define _my_tls (*((_cygtls *) ((char *)_tlsbase - CYGTLS_PADSIZE)))
extern _cygtls *_main_tls;
extern _cygtls *_sig_tls;

class san
{
  san *_clemente;
  jmp_buf _context;
  int _errno;
  unsigned _c_cnt;
  unsigned _w_cnt;
public:
  int setup (int myerrno = 0) __attribute__ ((always_inline))
  {
    _clemente = _my_tls.andreas;
    _my_tls.andreas = this;
    _errno = myerrno;
    _c_cnt = _my_tls.locals.pathbufs.c_cnt;
    _w_cnt = _my_tls.locals.pathbufs.w_cnt;
    return __sjfault (_context);
  }
  void leave () __attribute__ ((always_inline))
  {
    if (_errno)
      set_errno (_errno);
    /* Restore tls_pathbuf counters in case of error. */
    _my_tls.locals.pathbufs.c_cnt = _c_cnt;
    _my_tls.locals.pathbufs.w_cnt = _w_cnt;
    __ljfault (_context, 1);
  }
  void reset () __attribute__ ((always_inline))
  {
    _my_tls.andreas = _clemente;
  }
};

class myfault
{
  san sebastian;
public:
  ~myfault () __attribute__ ((always_inline)) { sebastian.reset (); }
  inline int faulted () __attribute__ ((always_inline))
  {
    return sebastian.setup (0);
  }
  inline int faulted (void const *obj, int myerrno = 0) __attribute__ ((always_inline))
  {
    return !obj || !(*(const char **) obj) || sebastian.setup (myerrno);
  }
  inline int faulted (int myerrno) __attribute__ ((always_inline))
  {
    return sebastian.setup (myerrno);
  }
};

class set_signal_arrived
{
public:
  set_signal_arrived (bool setit, HANDLE& h) { _my_tls.set_signal_arrived (setit, h); }
  set_signal_arrived (HANDLE& h) { _my_tls.set_signal_arrived (true, h); }

  operator int () const {return _my_tls.will_wait_for_signal;}
  ~set_signal_arrived () { _my_tls.reset_signal_arrived (); }
};

#define __getreent() (&_my_tls.local_clib)
/*gentls_offsets*/
