/* pwdgrp.h

   Copyright 2001, 2002, 2003 Red Hat inc.

   Stuff common to pwd and grp handling.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* These functions are needed to allow searching and walking through
   the passwd and group lists */
extern struct passwd *internal_getpwsid (cygsid &);
extern struct passwd *internal_getpwnam (const char *, bool = FALSE);
extern struct passwd *internal_getpwuid (__uid32_t, bool = FALSE);
extern struct __group32 *internal_getgrsid (cygsid &);
extern struct __group32 *internal_getgrgid (__gid32_t gid, bool = FALSE);
extern struct __group32 *internal_getgrnam (const char *, bool = FALSE);
extern struct __group32 *internal_getgrent (int);
int internal_getgroups (int, __gid32_t *, cygsid * = NULL);

enum pwdgrp_state {
  uninitialized = 0,
  initializing,
  loaded
};

class pwdgrp
{
  pwdgrp_state state;
  int pwd_ix;
  path_conv pc;
  char *buf;
  int max_lines;
  union
  {
    passwd **passwd_buf;
    __group32 **group_buf;
    void **pwdgrp_buf;
  };
  unsigned pwdgrp_buf_elem_size;
  bool (pwdgrp::*parse) (char *);

  char *gets (char*&);
  bool parse_pwd (char *);
  bool parse_grp (char *);

public:
  int curr_lines;

  void add_line (char *);
  bool isinitializing ()
  {
    if (state <= initializing)
      state = initializing;
    else if (etc::file_changed (pwd_ix))
      state = initializing;
    return state == initializing;
  }
  void operator = (pwdgrp_state nstate) { state = nstate; }
  bool isuninitialized () const { return state == uninitialized; }

  bool load (const char *);
  bool load (const char *posix_fname, passwd *&buf)
  {
    passwd_buf = &buf;
    pwdgrp_buf_elem_size = sizeof (*buf);
    parse = &pwdgrp::parse_pwd;
    return load (posix_fname);
  }
  bool load (const char *posix_fname, __group32 *&buf)
  {
    group_buf = &buf;
    pwdgrp_buf_elem_size = sizeof (*buf);
    parse = &pwdgrp::parse_grp;
    return load (posix_fname);
  }
};
