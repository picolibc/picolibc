/* childinfo.h: shared child info for cygwin

   Copyright 2000 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

enum
{
  PROC_MAGIC = 0xaf08f000,
  PROC_FORK = PROC_MAGIC + 1,
  PROC_EXEC = PROC_MAGIC + 2,
  PROC_SPAWN = PROC_MAGIC + 3,
  PROC_FORK1 = PROC_MAGIC + 4	// Newer versions provide stack
				// location information
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
  HANDLE myself_pinfo;
  ~child_info ()
  {
    if (myself_pinfo)
      CloseHandle (myself_pinfo);
  }
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

void __stdcall init_child_info (DWORD, child_info *, int, HANDLE);

extern child_info_fork *child_proc_info;

/* non-NULL if this process is a child of a cygwin process */
extern HANDLE parent_alive;
