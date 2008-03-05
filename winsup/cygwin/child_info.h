/* child_info.h: shared child info for cygwin

   Copyright 2000, 2001, 2002, 2003, 2004, 2005, 2006 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <setjmp.h>

enum child_info_types
{
  _PROC_EXEC,
  _PROC_SPAWN,
  _PROC_FORK,
  _PROC_WHOOPS
};

enum child_status
{
  _CI_STRACED	 = 0x01,
  _CI_ISCYGWIN	 = 0x02,
  _CI_SAW_CTRL_C = 0x04
};

#define OPROC_MAGIC_MASK 0xff00ff00
#define OPROC_MAGIC_GENERIC 0xaf00f000

#define PROC_MAGIC_GENERIC 0xaf00fa00

#define PROC_EXEC (_PROC_EXEC)
#define PROC_SPAWN (_PROC_SPAWN)
#define PROC_FORK (_PROC_FORK)

#define EXEC_MAGIC_SIZE sizeof(child_info)

/* Change this value if you get a message indicating that it is out-of-sync. */
#define CURR_CHILD_INFO_MAGIC 0xc3a86a0bU

/* NOTE: Do not make gratuitous changes to the names or organization of the
   below class.  The layout is checksummed to determine compatibility between
   different cygwin versions. */
class child_info
{
public:
  DWORD msv_count;	// zeroed on < W2K3, set to pseudo-count on Vista
  DWORD cb;		// size of this record
  DWORD intro;		// improbable string
  unsigned long magic;	// magic number unique to child_info
  unsigned short type;	// type of record, exec, spawn, fork
  HANDLE subproc_ready;	// used for synchronization with parent
  HANDLE user_h;
  HANDLE parent;
  init_cygheap *cygheap;
  void *cygheap_max;
  DWORD cygheap_reserve_sz;
  unsigned char flag;
  unsigned fhandler_union_cb;
  int retry;		// number of times we've tried to start child process
  DWORD exit_code;	// process exit code
  static int retry_count;// retry count;
  child_info (unsigned, child_info_types, bool);
  child_info (): subproc_ready (NULL), parent (NULL) {}
  ~child_info ();
  void ready (bool);
  bool sync (int, HANDLE&, DWORD) __attribute__ ((regparm (3)));
  DWORD proc_retry (HANDLE) __attribute__ ((regparm (2)));
  bool isstraced () const {return !!(flag & _CI_STRACED);}
  bool iscygwin () const {return !!(flag & _CI_ISCYGWIN);}
  bool saw_ctrl_c () const {return !!(flag & _CI_SAW_CTRL_C);}
  void set_saw_ctrl_c () {flag |= _CI_SAW_CTRL_C;}
};

class mount_info;
class _pinfo;

class child_info_fork: public child_info
{
public:
  HANDLE forker_finished;// for synchronization with child
  DWORD stacksize;	// size of parent stack
  jmp_buf jmp;		// where child will jump to
  void *stacktop;	// location of top of parent stack
  void *stackbottom;	// location of bottom of parent stack
  char filler[4];
  child_info_fork ();
  void handle_fork () __attribute__ ((regparm (1)));;
  bool handle_failure (DWORD) __attribute__ ((regparm (2)));
  void alloc_stack ();
  void alloc_stack_hard_way (volatile char *);
};

class fhandler_base;

class cygheap_exec_info
{
public:
  char *old_title;
  int argc;
  char **argv;
  int envc;
  char **envp;
  HANDLE myself_pinfo;
};

class child_info_spawn: public child_info
{
public:
  cygheap_exec_info *moreinfo;
  int __stdin;
  int __stdout;
  char filler[4];

  ~child_info_spawn ()
  {
    if (moreinfo)
      {
	if (moreinfo->old_title)
	  cfree (moreinfo->old_title);
	if (moreinfo->envp)
	  {
	    for (char **e = moreinfo->envp; *e; e++)
	      cfree (*e);
	    cfree (moreinfo->envp);
	  }
	if (moreinfo->myself_pinfo)
	  CloseHandle (moreinfo->myself_pinfo);
	cfree (moreinfo);
      }
  }
  child_info_spawn (): moreinfo (NULL) {};
  child_info_spawn (child_info_types, bool);
  void *operator new (size_t, void *p) __attribute__ ((nothrow)) {return p;}
  void set (child_info_types ci, bool b) { new (this) child_info_spawn (ci, b);}
  void handle_spawn () __attribute__ ((regparm (1)));
};

void __stdcall init_child_info (DWORD, child_info *, HANDLE);

extern "C" {
extern child_info *child_proc_info;
extern child_info_spawn *spawn_info asm ("_child_proc_info");
extern child_info_fork *fork_info asm ("_child_proc_info");
}
