/* childinfo.h: shared child info for cygwin

   Copyright 2000 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <setjmp.h>

enum
{
  _PROC_EXEC,
  _PROC_SPAWN,
  _PROC_FORK
};

#define OPROC_MAGIC_MASK 0xff00ff00
#define OPROC_MAGIC_GENERIC 0xaf00f000

#define PROC_MAGIC_GENERIC 0xaf00fa00

#define PROC_EXEC (_PROC_EXEC)
#define PROC_SPAWN (_PROC_SPAWN)
#define PROC_FORK (_PROC_FORK)

#define EXEC_MAGIC_SIZE sizeof(child_info)

#define CURR_CHILD_INFO_MAGIC 0x8b3c

/* NOTE: Do not make gratuitous changes to the names or organization of the
   below class.  The layout is checksummed to determine compatibility between
   different cygwin versions. */
class child_info
{
public:
  DWORD zero[4];	// must be zeroed
  DWORD cb;		// size of this record
  DWORD intro;		// improbable string
  unsigned short magic;	// magic number unique to child_info
  unsigned short type;	// type of record, exec, spawn, fork
  int cygpid;		// cygwin pid of child process
  HANDLE subproc_ready;	// used for synchronization with parent
  HANDLE mount_h;
  HANDLE parent;
  HANDLE pppid_handle;
  init_cygheap *cygheap;
  void *cygheap_max;
  HANDLE cygheap_h;
  unsigned fhandler_union_cb;
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
