/* pinfo.h: process table info

   Copyright 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* Signal constants (have to define them here, unfortunately) */

enum
{
  __SIGFLUSH	    = -2,
  __SIGSTRACE	    = -1,
  __SIGCHILDSTOPPED =  0,
  __SIGOFFSET	    =  3
};

#define PSIZE 1024

#include <sys/resource.h>
#include "thread.h"

class _pinfo
{
public:
  /* Cygwin pid */
  pid_t pid;

  /* Various flags indicating the state of the process.  See PID_
     constants below. */
  DWORD process_state;

  /* If hProcess is set, it's because it came from a
     CreateProcess call.  This means it's process relative
     to the thing which created the process.  That's ok because
     we only use this handle from the parent. */
  HANDLE hProcess;

#define PINFO_REDIR_SIZE ((DWORD) &(((_pinfo *)NULL)->hProcess) + sizeof (DWORD))

  /* Handle associated with initial Windows pid which started it all. */
  HANDLE pid_handle;

  /* Parent process id.  */
  pid_t ppid;

  /* dwProcessId contains the processid used for sending signals.  It
   * will be reset in a child process when it is capable of receiving
   * signals.
   */
  DWORD dwProcessId;

  /* Used to spawn a child for fork(), among other things. */
  char progname[MAX_PATH];

  HANDLE parent_alive;

  /* User information.
     The information is derived from the GetUserName system call,
     with the name looked up in /etc/passwd and assigned a default value
     if not found.  This data resides in the shared data area (allowing
     tasks to store whatever they want here) so it's for informational
     purposes only. */
  uid_t uid;		/* User ID */
  gid_t gid;		/* Group ID */
  pid_t pgid;		/* Process group ID */
  pid_t sid;		/* Session ID */
  int ctty;		/* Control tty */
  mode_t umask;
  char username[MAX_USER_NAME]; /* user's name */

  /* Extendend user information.
     The information is derived from the internal_getlogin call
     when on a NT system. */
  int use_psid;		/* TRUE if psid contains valid data */
  char psid[MAX_SID_LEN];  /* buffer for user's SID */
  char logsrv[MAX_HOST_NAME]; /* Logon server, may be FQDN */
  char domain[MAX_COMPUTERNAME_LENGTH+1]; /* Logon domain of the user */

  /* token is needed if sexec should be called. It can be set by a call
     to `set_impersonation_token()'. */
  HANDLE token;
  BOOL impersonated;
  uid_t orig_uid;	/* Remains intact also after impersonation */
  uid_t orig_gid;	/* Ditto */
  uid_t real_uid;	/* Remains intact on seteuid, replaced by setuid */
  gid_t real_gid;	/* Ditto */

  /* Filled when chroot() is called by the process or one of it's parents.
     Saved without trailing backslash. */
  char root[MAX_PATH+1];
  size_t rootlen;

  /* Resources used by process. */
  long start_time;
  struct rusage rusage_self;
  struct rusage rusage_children;
  /* Pointer to mmap'ed areas for this process.  Set up by fork. */
  void *mmap_ptr;

  /* Non-zero if process was stopped by a signal. */
  char stopsig;

  void exit (UINT n, bool norecord = 0) __attribute__ ((noreturn, regparm(2)));

  inline struct sigaction& getsig (int sig)
  {
    return thread2signal ? thread2signal->sigs[sig] : sigs[sig];
  }

  inline void copysigs (_pinfo *p) {sigs = p->sigs;}

  inline sigset_t& getsigmask ()
  {
    return thread2signal ? *thread2signal->sigmask : sig_mask;
  }

  inline void setsigmask (sigset_t mask)
  {
    if (thread2signal)
      *(thread2signal->sigmask) = mask;
    sig_mask = mask;
  }

  inline LONG* getsigtodo (int sig) {return _sigtodo + __SIGOFFSET + sig;}

  inline HANDLE getthread2signal ()
  {
    return thread2signal ? thread2signal->win32_obj_id : hMainThread;
  }

  inline void setthread2signal (void *thr) {thread2signal = (ThreadItem *) thr;}

private:
  struct sigaction sigs[NSIG];
  sigset_t sig_mask;		/* one set for everything to ignore. */
  LONG _sigtodo[NSIG + __SIGOFFSET];
  ThreadItem *thread2signal;  // NULL means means thread any other means a pthread
};

class pinfo
{
  HANDLE h;
  _pinfo *procinfo;
  int destroy;
public:
  void init (pid_t n, DWORD create = 0, HANDLE h = NULL);
  pinfo () {}
  pinfo (_pinfo *x): procinfo (x) {}
  pinfo (pid_t n) {init (n);}
  pinfo (pid_t n, int create) {init (n, create);}
  void release ();
  ~pinfo ()
  {
    if (destroy && procinfo)
      release ();
  }
    
  _pinfo *operator -> () const {return procinfo;}
  int operator == (pinfo *x) const {return x->procinfo == procinfo;}
  int operator == (pinfo &x) const {return x.procinfo == procinfo;}
  int operator == (void *x) const {return procinfo == x;}
  int operator == (int x) const {return (int) procinfo == (int) x;}
  int operator == (char *x) const {return (char *) procinfo == x;}
  _pinfo *operator * () const {return procinfo;}
  operator _pinfo * () const {return procinfo;}
  // operator bool () const {return (int) h;}
  void remember () {destroy = 0; proc_subproc (PROC_ADDCHILD, (DWORD) this);}
  HANDLE shared_handle () {return h;}
};

#define ISSTATE(p, f)	(!!((p)->process_state & f))
#define NOTSTATE(p, f)	(!((p)->process_state & f))

class winpids
{
  DWORD pidlist[16384];
public:
  DWORD npids;
  void reset () { npids = 0; }
  winpids (int) { reset (); }
  winpids () { init (); };
  void init ();
  int operator [] (int i) const {return pidlist[i];}
};

extern __inline pid_t
cygwin_pid (pid_t pid)
{
  return (pid_t) (os_being_run == winNT) ? pid : -(int) pid;
}

void __stdcall pinfo_init (char **, int);
void __stdcall set_myself (pid_t pid, HANDLE h = NULL);
extern pinfo myself;

#define _P_VFORK 0
extern "C" int _spawnve (HANDLE hToken, int mode, const char *path,
			 const char *const *argv, const char *const *envp);

extern void __stdcall pinfo_fixup_after_fork ();
extern HANDLE hexec_proc;

/* For mmaps across fork(). */
int __stdcall recreate_mmaps_after_fork (void *);
void __stdcall set_child_mmap_ptr (_pinfo *);

void __stdcall fill_rusage (struct rusage *, HANDLE);
void __stdcall add_rusage (struct rusage *, struct rusage *);
