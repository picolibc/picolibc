/* dtable.h: fd table definition.

   Copyright 2000, 2001, 2003 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* Initial and increment values for cygwin's fd table */
#define NOFILE_INCR    32

#include "thread.h"

class suffix_info;
class fhandler_fifo;

#define BFH_OPTS (PC_NULLEMPTY | PC_FULL | PC_POSIX)
class dtable
{
  fhandler_base **fds;
  fhandler_base **fds_on_hold;
  int first_fd_for_open;
  int cnt_need_fixup_before;
public:
  size_t size;

  dtable () : first_fd_for_open(3), cnt_need_fixup_before(0) {}
  void init () {first_fd_for_open = 3;}

  void dec_need_fixup_before ()
    { if (cnt_need_fixup_before > 0) --cnt_need_fixup_before; }
  void inc_need_fixup_before ()
    { cnt_need_fixup_before++; }
  bool need_fixup_before ()
    { return cnt_need_fixup_before > 0; }

  int vfork_child_dup ();
  void vfork_parent_restore ();
  void vfork_child_fixup ();
  fhandler_base *dup_worker (fhandler_base *oldfh);
  int extend (int howmuch);
  void fixup_before_exec (DWORD win_proc_id);
  void fixup_before_fork (DWORD win_proc_id);
  void fixup_after_fork (HANDLE);
  inline int not_open (int fd)
  {
    SetResourceLock (LOCK_FD_LIST, READ_LOCK, "not_open");

    int res = fd < 0 || fd >= (int) size || fds[fd] == NULL;

    ReleaseResourceLock (LOCK_FD_LIST, READ_LOCK, "not open");
    return res;
  }
  int find_unused_handle (int start);
  int find_unused_handle () { return find_unused_handle (first_fd_for_open);}
  void release (int fd);
  void init_std_file_from_handle (int fd, HANDLE handle);
  int dup2 (int oldfd, int newfd);
  void fixup_after_exec (HANDLE);
  inline fhandler_base *&operator [](int fd) const { return fds[fd]; }
  select_record *select_read (int fd, select_record *s);
  select_record *select_write (int fd, select_record *s);
  select_record *select_except (int fd, select_record *s);
  operator fhandler_base **() {return fds;}
  void stdio_init ();
  void get_debugger_info ();
  void set_file_pointers_for_exec ();
  bool in_vfork_cleanup () {return fds_on_hold == fds;}
  fhandler_fifo *find_fifo (ATOM);
};

fhandler_base *build_fh_dev (const device&, const char * = NULL);
fhandler_base *build_fh_name (const char *unix_name, HANDLE = NULL, unsigned = 0, suffix_info * = NULL);
fhandler_base *build_fh_pc (path_conv& pc);

void dtable_init (void);
void stdio_init (void);
extern dtable fdtab;

extern "C" int getfdtabsize ();
extern "C" void setfdtabsize (int);
