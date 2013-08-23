/* child_info.h: shared child info for cygwin

   Copyright 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2008, 2009, 2011, 2012,
   2013 Red Hat, Inc.

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
  _CI_SAW_CTRL_C = 0x04
};

#define OPROC_MAGIC_MASK 0xff00ff00
#define OPROC_MAGIC_GENERIC 0xaf00f000

#ifdef __x86_64__
#define PROC_MAGIC_GENERIC 0xaf00fa64
#else /*!x86_64*/
#define PROC_MAGIC_GENERIC 0xaf00fa32
#endif

#define EXEC_MAGIC_SIZE sizeof(child_info)

/* Change this value if you get a message indicating that it is out-of-sync. */
#define CURR_CHILD_INFO_MAGIC 0x93737edaU

#define NPROCS	256

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
  DWORD msv_count;	// zeroed on < W2K3, set to pseudo-count on Vista
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
  child_info (unsigned, child_info_types, bool);
  child_info (): subproc_ready (NULL), parent (NULL) {}
  ~child_info ();
  void refresh_cygheap () { cygheap_max = ::cygheap_max; }
  void ready (bool);
  bool __reg3 sync (int, HANDLE&, DWORD);
  DWORD __reg2 proc_retry (HANDLE);
  bool isstraced () const {return !!(flag & _CI_STRACED);}
  bool iscygwin () const {return !!(flag & _CI_ISCYGWIN);}
  bool saw_ctrl_c () const {return !!(flag & _CI_SAW_CTRL_C);}
  void prefork (bool = false);
  void cleanup ();
  void postfork (pinfo& child)
  {
    ForceCloseHandle (wr_proc_pipe);
    wr_proc_pipe = NULL;
    child.set_rd_proc_pipe (rd_proc_pipe);
    rd_proc_pipe = NULL;
  }
};

class mount_info;

class child_info_fork: public child_info
{
public:
  HANDLE forker_finished;// for synchronization with child
  jmp_buf jmp;		// where child will jump to
  void *stackaddr;	// address of parent stack
  void *stacktop;	// location of top of parent stack
  void *stackbottom;	// location of bottom of parent stack
  size_t guardsize;     // size of POSIX guard region or (size_t) -1 if
			// user stack
  char filler[4];
  child_info_fork ();
  void __reg1 handle_fork ();
  bool abort (const char *fmt = NULL, ...);
  void alloc_stack ();
  void alloc_stack_hard_way (volatile char *);
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
  sigset_t sigmask;
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
  void __reg1 handle_spawn ();
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
  int __reg3 worker (const char *, const char *const *, const char *const [], int,
	      int = -1, int = -1);;
};

extern child_info_spawn ch_spawn;

#define have_execed ch_spawn.has_execed ()
#define have_execed_cygwin ch_spawn.has_execed_cygwin ()

void __stdcall init_child_info (DWORD, child_info *, HANDLE);

extern "C" {
extern child_info *child_proc_info;
extern child_info_spawn *spawn_info asm (_SYMSTR (child_proc_info));
extern child_info_fork *fork_info asm (_SYMSTR (child_proc_info));
}
