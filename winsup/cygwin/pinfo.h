/* pinfo.h: process table info

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#pragma once

#include <sys/resource.h>
#include "thread.h"

union commune_result
{
  struct {
    char *s;
    size_t n;
  };
  struct {
    sigset_t pnd;
    sigset_t blk;
    sigset_t ign;
  };
};

enum picom
{
  PICOM_EXTRASTR = 0x80000000,
  PICOM_CMDLINE = 1,
  PICOM_CWD = 2,
  PICOM_ROOT = 3,
  PICOM_FDS = 4,
  PICOM_FD = 5,
  PICOM_PIPE_FHANDLER = 6,
  PICOM_FILE_PATHCONV = 7,
  PICOM_ENVIRON = 8,
  PICOM_SIGINFO = 9
};

#define EXITCODE_SET		0x8000000
#define EXITCODE_NOSET		0x4000000
#define EXITCODE_RETRY		0x2000000
#define EXITCODE_OK		0x1000000
#define EXITCODE_FORK_FAILED	0x0800000

class fhandler_pipe;

class _pinfo
{
public:
  /* Cygwin pid */
  pid_t pid;

  /* Various flags indicating the state of the process.  See PID_
     constants in <sys/cygwin.h>. */
  DWORD process_state;

  pid_t ppid;		/* Parent process id.  */

  DWORD exitcode;	/* set when process exits */

  /* > 0 if started by a cygwin process */
  DWORD cygstarted;

  /* dwProcessId contains the processid used for sending signals.  It
    will be reset in a child process when it is capable of receiving
    signals.  */
  DWORD dwProcessId;

  /* Used to spawn a child for fork(), among other things.  The other
     members of _pinfo take only a bit over 200 bytes.  So cut off a
     couple of bytes from progname to allow the _pinfo structure not
     to exceed 64K.  Otherwise it blocks another 64K block of VM for
     the process.  */
  WCHAR progname[NT_MAX_PATH - 512];

  /* User information.
     The information is derived from the GetUserName system call,
     with the name looked up in /etc/passwd and assigned a default value
     if not found.  This data resides in the shared data area (allowing
     tasks to store whatever they want here) so it's for informational
     purposes only. */
  uid_t uid;	/* User ID */
  gid_t gid;	/* Group ID */
  pid_t pgid;	/* Process group ID */
  pid_t sid;	/* Session ID */
  int ctty;	/* Control tty */
  bool has_pgid_children;/* True if we've forked or spawned children with our GID. */

  /* Resources used by process. */
  long start_time;
  struct rusage rusage_self;
  struct rusage rusage_children;
  int nice;

  /* Non-zero if process was stopped by a signal. */
  char stopsig;

  inline void set_has_pgid_children ()
  {
    if (pgid == pid)
      has_pgid_children = 1;
  }

  inline void set_has_pgid_children (bool val) {has_pgid_children = val;}

  commune_result commune_request (__uint32_t, ...);
  bool alive ();
  fhandler_pipe *pipe_fhandler (int64_t, size_t &);
  void *file_pathconv (int, uint32_t, size_t &);
  char *fd (int fd, size_t &);
  char *fds (size_t &);
  char *root (size_t &);
  char *cwd (size_t &);
  char *cmdline (size_t &);
  char *environ (size_t &);
  char *win_heap_info (size_t &);
  int siginfo (sigset_t &, sigset_t &, sigset_t &);
  bool set_ctty (class fhandler_termios *, int);
  bool alert_parent (char);
  int kill (siginfo_t&);
  bool exists ();
  const char *_ctty (char *);

  /* signals */
  HANDLE sendsig;
  HANDLE exec_sendsig;
  DWORD exec_dwProcessId;
public:
  friend class pinfo_minimal;
};

DWORD commune_process (void *);

enum parent_alerter
{
  __ALERT_REPARENT = 111,		// arbitrary non-signal value
  __ALERT_ALIVE   =  112
};

class pinfo_minimal
{
  HANDLE h;
public:
  HANDLE hProcess;
  HANDLE rd_proc_pipe;
  pinfo_minimal (): h (NULL), hProcess (NULL), rd_proc_pipe (NULL) {}
  void set_rd_proc_pipe (HANDLE& h) {rd_proc_pipe = h;}
  void set_inheritance (bool);
  friend class pinfo;
};

class pinfo: public pinfo_minimal
{
  bool destroy;
  HANDLE winpid_hdl;
  _pinfo *procinfo;
public:
  bool waiter_ready;
  class cygthread *wait_thread;

  void init (pid_t, DWORD, HANDLE);
  pinfo (_pinfo *x = NULL)
  : pinfo_minimal (), destroy (false), winpid_hdl (NULL), procinfo (x),
    waiter_ready (false), wait_thread (NULL) {}
  pinfo (pid_t n, DWORD flag = 0)
  : pinfo_minimal (), destroy (false), winpid_hdl (NULL), procinfo (NULL),
    waiter_ready (false), wait_thread (NULL)
  {
    init (n, flag, NULL);
  }
  pinfo (HANDLE, pinfo_minimal&, pid_t);
  void thisproc (HANDLE);
  void create_winpid_symlink ();
  inline void _pinfo_release ();
  void release ();
  bool wait ();
  ~pinfo ()
  {
    if (destroy && procinfo)
      release ();
  }
  void exit (DWORD n) __attribute__ ((noreturn, ));
  void maybe_set_exit_code_from_windows ();
  void set_exit_code (DWORD n);
  _pinfo *operator -> () const {return procinfo;}
  int operator == (pinfo *x) const {return x->procinfo == procinfo;}
  int operator == (pinfo &x) const {return x.procinfo == procinfo;}
  int operator == (_pinfo *x) const {return x == procinfo;}
  int operator == (void *x) const {return procinfo == x;}
  _pinfo *operator * () const {return procinfo;}
  operator _pinfo * () const {return procinfo;}
  int operator !() const {return !procinfo;}
  void preserve () { destroy = false; }
  void allow_remove () { destroy = true; }
#ifndef SIG_BAD_MASK		// kludge to ensure that sigproc.h included
  // int attach () {system_printf ("attach is not here"); return 0;}
  // int remember (bool) {system_printf ("remember is not here"); return 0;}
#else
  int attach ()
  {
    int res = proc_subproc (PROC_ATTACH_CHILD, (uintptr_t) this);
    destroy = res ? false : true;
    return res;
  }
  int remember ()
  {
    int res = proc_subproc (PROC_ADD_CHILD, (uintptr_t) this);
    destroy = res ? false : true;
    return res;
  }
#endif
  HANDLE shared_handle () {return h;}
  HANDLE shared_winpid_handle () {return winpid_hdl;}
  void set_acl ();
  friend class _pinfo;
  friend class winpids;
private:
  DWORD status_exit (DWORD);
};

#define MAX_PID 65536

#define ISSTATE(p, f)	(!!((p)->process_state & f))
#define NOTSTATE(p, f)	(!((p)->process_state & f))

class winpids
{
  bool make_copy;
  DWORD npidlist;
  DWORD *pidlist;
  pinfo *pinfolist;
  DWORD pinfo_access;		// access type for pinfo open
  DWORD enum_processes (bool winpid);
  DWORD enum_init (bool winpid);
  void add (DWORD& nelem, bool, DWORD pid);
public:
  DWORD npids;
  inline void reset () { release (); npids = 0;}
  void set (bool winpid);
  winpids (): make_copy (true) {}
  winpids (DWORD acc): make_copy (false), npidlist (0), pidlist (NULL),
		       pinfolist (NULL), pinfo_access (acc), npids (0)
  {
    set (0);
  }
  inline DWORD& winpid (int i) const {return pidlist[i];}
  inline _pinfo *operator [] (int i) const {return (_pinfo *) pinfolist[i];}
  ~winpids ();
  void release ();
};

pid_t create_cygwin_pid ();
pid_t cygwin_pid (DWORD);

void pinfo_init (char **, int);
extern pinfo myself;

/* Helper class to allow convenient setting and unsetting a process_state
   flag in myself.  This is used in certain fhandler read/write methods
   to set the PID_TTYIN/PID_TTYOU flags in myself->process_state. */
class push_process_state
{
private:
  int flag;
public:
  push_process_state (int add_flag)
  {
    flag = add_flag;
    myself->process_state |= flag;
  }
  void pop () { myself->process_state &= ~(flag); }
  ~push_process_state () { pop (); }
};

#define _P_VFORK 0
#define _P_SYSTEM 512
/* Add this flag in calls to child_info_spawn::worker if the calling function
   is one of 'p' type functions: execlp, execvp, spawnlp, spawnvp.  Per POSIX,
   only these p-type functions fall back to call /bin/sh if the file is not a
   binary.  The setting of _P_PATH_TYPE_EXEC is used as a bool value in
   av::fixup to decide if the file should be evaluated as a script, or if
   ENOEXEC should be returned. */
#define _P_PATH_TYPE_EXEC	0x1000

/* Helper macro to mask actual mode and drop additional flags defined above. */
#define _P_MODE(x)		((x) & 0xfff)

#define __ctty() _ctty ((char *) alloca (sizeof ("ctty /dev/tty") + 20))
#define myctty() myself->__ctty ()

/* For mmaps across fork(). */
int fixup_mmaps_after_fork (HANDLE parent);
/* for shm areas across fork (). */
int fixup_shms_after_fork ();

void fill_rusage (struct rusage *, HANDLE);
void add_rusage (struct rusage *, struct rusage *);
