/******** Process Table ********/

/* Signal constants (have to define them here, unfortunately) */

enum
{
  __SIGFLUSH	    = -2,
  __SIGSTRACE	    = -1,
  __SIGCHILDSTOPPED =  0,
  __SIGOFFSET	    =  3
};

#define PSIZE 1024
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
  uid_t orig_uid;        /* Remains intact also after impersonation */
  uid_t orig_gid;        /* Ditto */
  uid_t real_uid;        /* Remains intact on seteuid, replaced by setuid */
  gid_t real_gid;	 /* Ditto */

  /* Filled when chroot() is called by the process or one of it's parents.
     Saved without trailing backslash. */
  char root[MAX_PATH+1];
  size_t rootlen;

  /* Non-zero if process was stopped by a signal. */
  char stopsig;

  struct sigaction& getsig (int);
  void copysigs (_pinfo* );
  sigset_t& getsigmask ();
  void setsigmask (sigset_t);
  LONG* getsigtodo (int);
  HANDLE getthread2signal ();
  void setthread2signal (void *);

  /* Resources used by process. */
  long start_time;
  struct rusage rusage_self;
  struct rusage rusage_children;

private:
  struct sigaction sigs[NSIG];
  sigset_t sig_mask;		/* one set for everything to ignore. */
  LONG _sigtodo[NSIG + __SIGOFFSET];
  ThreadItem* thread2signal;  // NULL means means thread any other means a pthread

public:
  /* Pointer to mmap'ed areas for this process.  Set up by fork. */
  void *mmap_ptr;

  void record_death ();
};

class pinfo
{
  HANDLE h;
  _pinfo *child;
  int destroy;
public:
  void init (pid_t n, DWORD create = 0);
  pinfo () {}
  pinfo (_pinfo *x): child (x) {}
  pinfo (pid_t n) {init (n);}
  pinfo (pid_t n, int create) {init (n, create);}
  void release ()
  {
    if (h)
      {
	UnmapViewOfFile (child);
	CloseHandle (h);
	h = NULL;
      }
  }
  ~pinfo ()
  {
    if (destroy && child)
      release ();
  }
    
  _pinfo *operator -> () const {return child;}
  int operator == (pinfo *x) const {return x->child == child;}
  int operator == (pinfo &x) const {return x.child == child;}
  int operator == (void *x) const {return child == x;}
  int operator == (int x) const {return (int) child == (int) x;}
  int operator == (char *x) const {return (char *) child == x;}
  _pinfo *operator * () const {return child;}
  operator _pinfo * () const {return child;}
  void remember () {destroy = 0; proc_subproc (PROC_ADDCHILD, (DWORD) this);}
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
void __stdcall pinfo_init (PBYTE);
