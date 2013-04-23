/* fhandler_virtual.h: Header for virtual fhandlers

   Copyright 2009, 2010, 2011 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

struct virt_tab_t {
  const char *name;
  size_t name_len;
  fh_devices fhandler;
  virtual_ftype_t type;
  off_t (*format_func)(void *data, char *&);
};

#define _VN(s)	s, sizeof (s) - 1

extern virt_tab_t *virt_tab_search (const char *, bool, const virt_tab_t *,
				    size_t);
