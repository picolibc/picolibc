/* childinfo.h: shared child info for cygwin

   Copyright 2000 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <setjmp.h>

enum
{
  PROC_MAGIC = 0xaf0af000,
  PROC_FORK = PROC_MAGIC + 1,
  PROC_EXEC = PROC_MAGIC + 2,
  PROC_SPAWN = PROC_MAGIC + 3,
  PROC_FORK1 = PROC_MAGIC + 4,	// Newer versions provide stack
				// location information
  PROC_SPAWN1 = PROC_MAGIC + 5
};

#define PROC_MAGIC_MASK 0xff00f000
#define PROC_MAGIC_GENERIC 0xaf00f000
#define PROC_MAGIC_VER_MASK 0x0ff0000

#define EXEC_MAGIC_SIZE sizeof(child_info)
class child_info
{
public:
  DWORD zero[1];	// must be zeroed
  DWORD cb;		// size of this record
  DWORD type;		// type of record
  int cygpid;		// cygwin pid of child process
  HANDLE subproc_ready;	// used for synchronization with parent
  HANDLE shared_h;
  HANDLE console_h;
  HANDLE parent_alive;	// handle of thread used to track children
  HANDLE parent;
  HANDLE pppid_handle;
  init_cygheap *cygheap;
  void *cygheap_max;
};

class child_info_fork: public child_info
{
public:
  HANDLE forker_finished;// for synchronization with child
  DWORD stacksize;	// size of parent stack
  void *heaptop;
  void *heapbase;
  void *heapptr;
  jmp_buf jmp;		// where child will jump to
  void *stacktop;	// location of top of parent stack
  void *stackbottom;	// location of bottom of parent stack
};

class fhandler_base;

class cygheap_exec_info
{
public:
  uid_t uid;
  char *old_title;
  fhandler_base **fds;
  size_t nfds;
  int argc;
  char **argv;
  int envc;
  char **envp;
  HANDLE myself_pinfo;
  char *cwd_posix;
  char *cwd_win32;
  DWORD cwd_hash;
};

class child_info_spawn: public child_info
{
public:
  cygheap_exec_info *moreinfo;
  HANDLE hexec_proc;

  child_info_spawn (): moreinfo (NULL) {}
  ~child_info_spawn ()
  {
    if (parent)
      CloseHandle (parent);
    if (moreinfo)
      {
	if (moreinfo->old_title)
	  cfree (moreinfo->old_title);
	if (moreinfo->cwd_posix)
	  cfree (moreinfo->cwd_posix);
	if (moreinfo->cwd_win32)
	  cfree (moreinfo->cwd_win32);
	if (moreinfo->envp)
	  {
	    for (char **e = moreinfo->envp; *e; e++)
	      cfree (*e);
	    cfree (moreinfo->envp);
	  }
	CloseHandle (moreinfo->myself_pinfo);
	cfree (moreinfo);
      }
  }
};

void __stdcall init_child_info (DWORD, child_info *, int, HANDLE);

extern child_info_fork *child_proc_info;
