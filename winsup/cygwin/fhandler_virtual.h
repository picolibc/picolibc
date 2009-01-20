/* fhandler_virtual.h: Header for virtual fhandlers

   Copyright 2009 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

enum virtual_ftype_t {
  virt_socket = -4,
  virt_pipe = -3,
  virt_symlink = -2,
  virt_file = -1,
  virt_none = 0,
  virt_directory = 1,
  virt_rootdir = 2
};

struct virt_tab_t {
  const char *name;
  __dev32_t fhandler;
  virtual_ftype_t type;
  _off64_t (*format_func)(void *data, char *&);
};

