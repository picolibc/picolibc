/* cygtls.h

   Copyright 2003 Red Hat, Inc.

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

#define CYGTLS_INITIALIZED 0x43227
#define CYGTLS_EXCEPTION (0x43227 + true)

#ifndef CYG_MAX_PATH
# define CYG_MAX_PATH 260
#endif

#ifndef UNLEN
# define UNLEN 256
#endif

#define TLS_STACK_SIZE 256

#pragma pack(push,4)
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

  /* strerror */
  char strerror_buf[20];

  /* sysloc.cc */
  char *process_ident;
  int process_logopt;
  int process_facility;
  int process_logmask;

  /* times.cc */
  char timezone_buf[20];
  struct tm _localtime_buf;

  /* uinfo.cc */
  char username[UNLEN + 1];

  /* net.cc */
  char *ntoa_buf;
  struct protoent *protoent_buf;
  struct servent *servent_buf;
  struct hostent *hostent_buf;
};

/* Please keep this file simple.  Changes to the below structure may require
   acompanying changes to the very simple parser in the perl script
   'gentls_offsets' (<<-- start parsing here).  */

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
  struct _reent local_clib;
  struct _local_storage locals;
  struct _threadinfo *prev, *next;
  __stack_t *stackptr;
  int sig;
  __stack_t stack[TLS_STACK_SIZE];
  unsigned padding[0];

  /*gentls_offsets*/
  static CRITICAL_SECTION protect_linked_list;
  static void init ();
  void init_thread (void *, DWORD (*) (void *, void *));
  static void call (DWORD (*) (void *, void *), void *) __attribute__ ((regparm (3)));
  static void call2 (DWORD (*) (void *, void *), void *, void *) __attribute__ ((regparm (3)));
  static struct _threadinfo *find_tls (int sig);
  void remove (DWORD);
  void push (__stack_t, bool = false);
  __stack_t pop ();
  bool isinitialized () {return initialized == CYGTLS_INITIALIZED || initialized == CYGTLS_EXCEPTION;}
  void set_state (bool);
  void reset_exception ();
  bool interrupt_now (CONTEXT *, int, void *, struct sigaction&)
    __attribute__((regparm(3)));
  void __stdcall interrupt_setup (int sig, void *handler, struct sigaction& siga, __stack_t retaddr)
    __attribute__((regparm(3)));
  void init_threadlist_exceptions (struct _exception_list *);
  operator HANDLE () const {return tid->win32_obj_id;}
  /*gentls_offsets*/
};
#pragma pack(pop)

extern char *_tlsbase __asm__ ("%fs:4");
extern char *_tlstop __asm__ ("%fs:8");
#define _my_tls (((_threadinfo *) _tlsbase)[-1])
extern _threadinfo *_main_tls;

#define __getreent() (&_my_tls.local_clib)

const int CYGTLS_PADSIZE  = (((char *) _main_tls->padding) - ((char *) _main_tls));
#endif /*_CYGTLS_H*/
