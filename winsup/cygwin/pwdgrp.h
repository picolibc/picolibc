/* pwdgrp.h

   Copyright 2001, 2002, 2003 Red Hat inc.

   Stuff common to pwd and grp handling.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* These functions are needed to allow searching and walking through
   the passwd and group lists */
extern struct passwd *internal_getpwsid (cygpsid &);
extern struct passwd *internal_getpwnam (const char *, bool = FALSE);
extern struct passwd *internal_getpwuid (__uid32_t, bool = FALSE);
extern struct __group32 *internal_getgrsid (cygpsid &);
extern struct __group32 *internal_getgrgid (__gid32_t gid, bool = FALSE);
extern struct __group32 *internal_getgrnam (const char *, bool = FALSE);
extern struct __group32 *internal_getgrent (int);
int internal_getgroups (int, __gid32_t *, cygpsid * = NULL);

#include "sync.h"
#include "cygtls.h"
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
  UNICODE_STRING upath;
  PWCHAR path;
  char *buf, *lptr;
  int max_lines;
  bool initialized;
  static muto pglock;

  bool parse_passwd ();
  bool parse_group ();
  void read_passwd ();
  void read_group ();
  char *add_line (char *);
  char *raw_ptr () const {return lptr;}
  char *next_str (char);
  bool next_num (unsigned long&);
  bool next_num (unsigned int& i)
  {
    unsigned long x;
    bool res = next_num (x);
    i = (unsigned int) x;
    return res;
  }
  inline bool next_num (int& i)
  {
    unsigned long x;
    bool res = next_num (x);
    i = (int) x;
    return res;
  }

public:
  int curr_lines;

  void load (const wchar_t *);
  inline void refresh (bool check)
  {
    if (!check && initialized)
      return;
    if (pglock.acquire () == 1 &&
	(!initialized || (check && etc::file_changed (etc_ix))))
	(this->*read) ();
    pglock.release ();
  }

  pwdgrp (passwd *&pbuf);
  pwdgrp (__group32 *&gbuf);
};
