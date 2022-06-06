/* child_info.h: shared child info for cygwin

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <setjmp.h>

enum child_info_types
{
  _CH_NADA = 0,
  _CH_EXEC = 1,
  _CH_SPAWN = 2,
  _CH_FORK = 3,
  _CH_WHOOPS = 4
};

enum child_status
{
  _CI_STRACED	 = 0x01,
  _CI_ISCYGWIN	 = 0x02,
  _CI_SAW_CTRL_C = 0x04,
  _CI_SILENTFAIL = 0x08
};

#define OPROC_MAGIC_MASK 0xff00ff00
#define OPROC_MAGIC_GENERIC 0xaf00f000

#define PROC_MAGIC_GENERIC 0xaf00fa64

#define EXEC_MAGIC_SIZE sizeof(child_info)

/* Change this value if you get a message indicating that it is out-of-sync. */
#define CURR_CHILD_INFO_MAGIC 0xacbf4682U

#include "pinfo.h"
struct cchildren
{
  pid_t pid;
  pinfo_minimal p;
};

/* NOTE: Do not make gratuitous changes to the names or organization of the
   below class.  The layout is checksummed to determine compatibility between
   different cygwin versions. */
class child_info
{
public:
  DWORD msv_count;	// set to 0
  DWORD cb;		// size of this record
  DWORD intro;		// improbable string
  DWORD magic;		// magic number unique to child_info
  unsigned short type;	// type of record, exec, spawn, fork
  init_cygheap *cygheap;
  void *cygheap_max;
  unsigned char flag;
  int retry;		// number of times we've tried to start child process
  HANDLE rd_proc_pipe;
  HANDLE wr_proc_pipe;
  HANDLE subproc_ready;	// used for synchronization with parent
  HANDLE user_h;
  HANDLE parent;
  DWORD parent_winpid;
  DWORD cygheap_reserve_sz;
  unsigned fhandler_union_cb;
  DWORD exit_code;	// process exit code
  static int retry_count;// retry count;
  sigset_t sigmask;

  child_info (unsigned, child_info_types, bool);
  child_info (): subproc_ready (NULL), parent (NULL) {}
  ~child_info ();
  void refresh_cygheap () { cygheap_max = ::cygheap_max; }
  void ready (bool);
  bool sync (int, HANDLE&, DWORD);
  DWORD proc_retry (HANDLE);
  bool isstraced () const {return !!(flag & _CI_STRACED);}
  bool iscygwin () const {return !!(flag & _CI_ISCYGWIN);}
  bool saw_ctrl_c () const {return !!(flag & _CI_SAW_CTRL_C);}
  bool silentfail () const {return !!(flag & _CI_SILENTFAIL);}
  void prefork (bool = false);
  void cleanup ();
  void postfork (pinfo& child)
  {
    ForceCloseHandle (wr_proc_pipe);
    wr_proc_pipe = NULL;
    child.set_rd_proc_pipe (rd_proc_pipe);
    rd_proc_pipe = NULL;
  }
  void silentfail (bool f)
  {
    if (f)
      flag |= _CI_SILENTFAIL;
    else
      flag &= ~_CI_SILENTFAIL;
  }
};

class mount_info;

class child_info_fork: public child_info
{
public:
  HANDLE forker_finished;// for synchronization with child
  jmp_buf jmp;		// where child will jump to
  void *stackaddr;	// DeallocationStack or user-provided allocation address
			// of parent thread
  void *stacklimit;	// StackLimit of parent thread
  void *stackbase;	// StackBase of parent thread
  size_t guardsize;     // size of POSIX guard region or (size_t) -1 if
			// user stack
  char filler[4];
  child_info_fork ();
  void handle_fork ();
  bool abort (const char *fmt = NULL, ...);
  void alloc_stack ();
};

class fhandler_base;

class cygheap_exec_info
{
public:
  int argc;
  char **argv;
  int envc;
  char **envp;
  HANDLE myself_pinfo;
  int nchildren;
  cchildren children[0];
  static cygheap_exec_info *alloc ();
  void record_children ();
  void reattach_children (HANDLE);
};

class child_info_spawn: public child_info
{
  HANDLE hExeced;
  HANDLE ev;
  HANDLE sem;
  pid_t cygpid;
public:
  cygheap_exec_info *moreinfo;
  int __stdin;
  int __stdout;
  char filler[4];

  void cleanup ();
  child_info_spawn () {};
  child_info_spawn (child_info_types, bool);
  void record_children ();
  void reattach_children ();
  void *operator new (size_t, void *p) __attribute__ ((nothrow)) {return p;}
  void set (child_info_types ci, bool b) { new (this) child_info_spawn (ci, b);}
  void handle_spawn ();
  void set_sem (HANDLE _sem)
  {
    /* Don't leak semaphore handle into exec'ed process. */
    SetHandleInformation (sem = _sem, HANDLE_FLAG_INHERIT, 0);
  }
  bool set_saw_ctrl_c ()
  {
    if (!has_execed ())
      return false;
    flag |= _CI_SAW_CTRL_C;
    return true;
  }
  bool signal_myself_exited ()
  {
    if (!ev)
      return false;
    else
      {
	SetEvent (ev);
	return true;
      }
  }
  void wait_for_myself ();
  bool has_execed () const
  {
    if (hExeced)
      return true;
    if (type != _CH_EXEC)
      return false;
    return !!hExeced;
  }
  bool get_parent_handle ();
  bool has_execed_cygwin () const { return iscygwin () && has_execed (); }
  operator HANDLE& () {return hExeced;}
  int worker (const char *, const char *const *, const char *const [],
		     int, int = -1, int = -1);
};

extern child_info_spawn ch_spawn;

#define have_execed ch_spawn.has_execed ()
#define have_execed_cygwin ch_spawn.has_execed_cygwin ()

extern "C" {
extern child_info *child_proc_info;
extern child_info_spawn *spawn_info asm (_SYMSTR (child_proc_info));
extern child_info_fork *fork_info asm (_SYMSTR (child_proc_info));
}
