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

#include "sync.h"
class pwdgrp
{
  unsigned pwdgrp_buf_elem_size;
  union
  {
    passwd **passwd_buf;
    __group32 **group_buf;
    void **pwdgrp_buf;
  };
  void (pwdgrp::*read) ();
  bool (pwdgrp::*parse) ();
  int etc_ix;
  path_conv pc;
  char *buf, *lptr;
  int max_lines;
  bool initialized;
  muto *pglock;

  bool parse_passwd ();
  bool parse_group ();
  void read_passwd ();
  void read_group ();
  char *add_line (char *);
  char *raw_ptr () const {return lptr;}
  char *next_str (char = 0);
  bool next_num (unsigned long&);
  bool next_num (unsigned int& i)
  {
    unsigned long x;
    bool res = next_num (x);
    i = (unsigned int) x;
    return res;
  }
  bool next_num (int& i)
  {
    unsigned long x;
    bool res = next_num (x);
    i = (int) x;
    return res;
  }

public:
  int curr_lines;

  void load (const char *);
  void refresh (bool check = true)
  {
    if (!check && initialized)
      return;
    pglock->acquire ();
    if (!initialized || (check && etc::file_changed (etc_ix)))
      (this->*read) ();
    pglock->release ();
  }

  pwdgrp (passwd *&pbuf) :
    pwdgrp_buf_elem_size (sizeof (*pbuf)), passwd_buf (&pbuf)
    {
      read = &pwdgrp::read_passwd;
      parse = &pwdgrp::parse_passwd;
      new_muto (pglock);
    }
  pwdgrp (__group32 *&gbuf) :
    pwdgrp_buf_elem_size (sizeof (*gbuf)), group_buf (&gbuf)
    {
      read = &pwdgrp::read_group;
      parse = &pwdgrp::parse_group;
      new_muto (pglock);
    }
};
