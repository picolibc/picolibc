/* dtable.h: fd table definition.

   Copyright 2000 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* Initial and increment values for cygwin's fd table */
#define NOFILE_INCR    32

class dtable
{
  fhandler_base **fds;
  fhandler_base **fds_on_hold;
  int first_fd_for_open;
public:
  size_t size;
  dtable () {first_fd_for_open = 3;}
  int vfork_child_dup ();
  void vfork_parent_restore ();
  fhandler_base *dup_worker (fhandler_base *oldfh);
  int extend (int howmuch);
  void fixup_after_fork (HANDLE);
  fhandler_base *build_fhandler (int fd, DWORD dev, const char *name,
				 int unit = -1);
  fhandler_base *build_fhandler (int fd, const char *name, HANDLE h);
  int not_open (int n) __attribute__ ((regparm(1)));
  int find_unused_handle (int start);
  int find_unused_handle () { return find_unused_handle (first_fd_for_open);}
  void release (int fd);
  void init_std_file_from_handle (int fd, HANDLE handle, DWORD access, const char *name);
  int dup2 (int oldfd, int newfd);
  void fixup_after_exec (HANDLE, size_t, fhandler_base **);
  inline fhandler_base *operator [](int fd) { return fds[fd]; }
  select_record *select_read (int fd, select_record *s);
  select_record *select_write (int fd, select_record *s);
  select_record *select_except (int fd, select_record *s);
  operator fhandler_base **() {return fds;}
};

void dtable_init (void);
void stdio_init (void);
extern dtable fdtab;

extern "C" int getfdtabsize ();
extern "C" void setfdtabsize (int);
