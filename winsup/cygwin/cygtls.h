/* cygtls.h

   Copyright 2003, 2004, 2005, 2008 Red Hat, Inc.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGTLS_H
#define _CYGTLS_H

#include <signal.h>
#include <pwd.h>
#include <grp.h>
#include <sys/time.h>
#define _NOMNTENT_FUNCS
#include <mntent.h>
#undef _NOMNTENT_FUNCS
#include <setjmp.h>
#include <exceptions.h>

#define CYGTLS_INITIALIZED 0xc763173f

#ifndef CYG_MAX_PATH
# define CYG_MAX_PATH 260
#endif

#ifndef UNLEN
# define UNLEN 256
#endif

#define TLS_STACK_SIZE 256

#include "cygthread.h"

#define TP_NUM_C_BUFS 10
#define TP_NUM_W_BUFS 10

#pragma pack(push,4)
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
  /*
     Needed for the group functions
   */
  struct __group16 grp;
  char *namearray[2];
  int grp_pos;

  /* console.cc */
  unsigned rarg;

  /* dlfcn.cc */
  int dl_error;
  char dl_buffer[256];

  /* passwd.cc */
  struct passwd res;
  char pass[_PASSWORD_LEN];
  int pw_pos;

  /* path.cc */
  struct mntent mntbuf;
  int iteration;
  unsigned available_drives;
  char mnt_type[80];
  char mnt_opts[80];
  char mnt_fsname[CYG_MAX_PATH];
  char mnt_dir[CYG_MAX_PATH];

  /* select.cc */
  HANDLE select_sockevt;

  /* strerror */
  char strerror_buf[sizeof ("Unknown error 4294967295")];

  /* sysloc.cc */
  char *process_ident;			// note: malloced
  int process_logopt;
  int process_facility;
  int process_logmask;

  /* times.cc */
  char timezone_buf[20];
  struct tm _localtime_buf;

  /* uinfo.cc */
  char username[UNLEN + 1];

  /* net.cc */
  char *ntoa_buf;			// note: malloced
  char signamebuf[sizeof ("Unknown signal 4294967295   ")];

  unionent *hostent_buf;		// note: malloced
  unionent *protoent_buf;		// note: malloced
  unionent *servent_buf;		// note: malloced

  /* cygthread.cc */
  char unknown_thread_name[30];

  /* syscalls.cc */
  int setmode_file;
  int setmode_mode;

  /* All functions requiring temporary path buffers. */
  tls_pathbuf pathbufs;
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

typedef struct
{
  void *_myfault;
  int _myfault_errno;
  int _myfault_c_cnt;
  int _myfault_w_cnt;
} san;

/* Changes to the below structure may require acompanying changes to the very
   simple parser in the perl script 'gentls_offsets' (<<-- start parsing here).
   The union in this structure is used to force alignment between the version
   of the compiler used to generate tlsoffsets.h and the cygwin cross compiler.
*/

/*gentls_offsets*/
#include "cygerrno.h"

extern "C" int __sjfault (jmp_buf);
extern "C" int __ljfault (jmp_buf, int);

/*gentls_offsets*/

typedef __uint32_t __stack_t;
struct _cygtls
{
  void (*func) /*gentls_offsets*/(int)/*gentls_offsets*/;
  exception_list el;
  int saved_errno;
  int sa_flags;
  sigset_t oldmask;
  sigset_t deltamask;
  HANDLE event;
  int *errno_addr;
  sigset_t sigmask;
  sigset_t sigwait_mask;
  siginfo_t *sigwait_info;
  struct ucontext thread_context;
  DWORD thread_id;
  unsigned threadkill;
  siginfo_t infodata;
  struct pthread *tid;
  union
  {
    struct _reent local_clib;
    char __dontuse[8 * ((sizeof(struct _reent) + 4) / 8)];
  };
  struct _local_storage locals;
  class cygthread *_ctinfo;
  san andreas;
  waitq wq;
  struct _cygtls *prev, *next;
  int sig;
  unsigned incyg;
  unsigned spinning;
  unsigned stacklock;
  __stack_t *stackptr;
  __stack_t stack[TLS_STACK_SIZE];
  unsigned initialized;

  /*gentls_offsets*/
  static CRITICAL_SECTION protect_linked_list;
  static void init ();
  void init_thread (void *, DWORD (*) (void *, void *));
  static void call (DWORD (*) (void *, void *), void *);
  void call2 (DWORD (*) (void *, void *), void *, void *) __attribute__ ((regparm (3)));
  static struct _cygtls *find_tls (int sig);
  void remove (DWORD);
  void push (__stack_t) __attribute__ ((regparm (2)));
  __stack_t pop () __attribute__ ((regparm (1)));
  __stack_t retaddr () {return stackptr[-1];}
  bool isinitialized () const
  {
    volatile char here;
    return ((char *) this > &here) && initialized == CYGTLS_INITIALIZED;
  }
  bool interrupt_now (CONTEXT *, int, void *, struct sigaction&)
    __attribute__((regparm(3)));
  void __stdcall interrupt_setup (int sig, void *handler,
				  struct sigaction& siga)
    __attribute__((regparm(3)));

  /* exception handling */
  static int handle_exceptions (EXCEPTION_RECORD *, exception_list *, CONTEXT *, void *);
  bool inside_kernel (CONTEXT *);
  void init_exception_handler (int (*) (EXCEPTION_RECORD *, exception_list *, CONTEXT *, void*));
  void signal_exit (int) __attribute__ ((noreturn, regparm(2)));
  void copy_context (CONTEXT *) __attribute__ ((regparm(2)));
  void signal_debugger (int) __attribute__ ((regparm(2)));

#ifdef _THREAD_H
  operator HANDLE () const {return tid->win32_obj_id;}
#endif
  void set_siginfo (struct sigpacket *) __attribute__ ((regparm (3)));
  void set_threadkill () {threadkill = true;}
  void reset_threadkill () {threadkill = false;}
  int call_signal_handler () __attribute__ ((regparm (1)));
  void remove_wq (DWORD) __attribute__ ((regparm (1)));
  void fixup_after_fork () __attribute__ ((regparm (1)));
  void lock () __attribute__ ((regparm (1)));
  void unlock () __attribute__ ((regparm (1)));
  bool locked () __attribute__ ((regparm (1)));
  void*& fault_guarded () {return andreas._myfault;}
  void return_from_fault ()
  {
    if (andreas._myfault_errno)
      set_errno (andreas._myfault_errno);
    /* Restore tls_pathbuf counters in case of error. */
    locals.pathbufs.c_cnt = andreas._myfault_c_cnt;
    locals.pathbufs.w_cnt = andreas._myfault_w_cnt;
    __ljfault ((int *) andreas._myfault, 1);
  }
  int setup_fault (jmp_buf j, san& old_j, int myerrno) __attribute__ ((always_inline))
  {
    old_j._myfault = andreas._myfault;
    old_j._myfault_errno = andreas._myfault_errno;
    old_j._myfault_c_cnt = andreas._myfault_c_cnt;
    old_j._myfault_w_cnt = andreas._myfault_w_cnt;
    andreas._myfault = (void *) j;
    andreas._myfault_errno = myerrno;
    /* Save tls_pathbuf counters. */
    andreas._myfault_c_cnt = locals.pathbufs.c_cnt;
    andreas._myfault_w_cnt = locals.pathbufs.w_cnt;
    return __sjfault (j);
  }
  void reset_fault (san& old_j) __attribute__ ((always_inline))
  {
    andreas._myfault = old_j._myfault;
    andreas._myfault_errno = old_j._myfault_errno;
  }
  /*gentls_offsets*/
};
#pragma pack(pop)

const int CYGTLS_PADSIZE = 12700;	/* FIXME: Find some way to autogenerate
					   this value */
/*gentls_offsets*/

extern char *_tlsbase __asm__ ("%fs:4");
extern char *_tlstop __asm__ ("%fs:8");
#define _my_tls (*((_cygtls *) (_tlsbase - CYGTLS_PADSIZE)))
extern _cygtls *_main_tls;
extern _cygtls *_sig_tls;

class myfault
{
  jmp_buf buf;
  san sebastian;
public:
  ~myfault () __attribute__ ((always_inline)) { _my_tls.reset_fault (sebastian); }
  inline int faulted (int myerrno = 0) __attribute__ ((always_inline))
  {
    return _my_tls.setup_fault (buf, sebastian, myerrno);
  }
};

#define __getreent() (&_my_tls.local_clib)

#endif /*_CYGTLS_H*/ /*gentls_offsets*/
