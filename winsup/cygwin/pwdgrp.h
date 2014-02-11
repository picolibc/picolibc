/* pwdgrp.h

   Copyright 2001, 2002, 2003, 2014 Red Hat inc.

   Stuff common to pwd and grp handling.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#pragma once

/* These functions are needed to allow searching and walking through
   the passwd and group lists */
extern struct passwd *internal_getpwsid (cygpsid &);
extern struct passwd *internal_getpwnam (const char *);
extern struct passwd *internal_getpwuid (uid_t);
extern struct group *internal_getgrsid (cygpsid &);
extern struct group *internal_getgrgid (gid_t);
extern struct group *internal_getgrnam (const char *);
int internal_getgroups (int, gid_t *, cygpsid * = NULL);

#include "sync.h"

enum fetch_user_arg_type_t {
  SID_arg,
  NAME_arg,
  ID_arg
};

struct fetch_user_arg_t
{
  fetch_user_arg_type_t type;
  union {
    cygpsid *sid;
    const char *name;
    uint32_t id;
  };
  /* Only used in fetch_account_from_file/line. */
  size_t len;
};

struct pg_pwd
{
  struct passwd p;
  cygsid sid;
};

struct pg_grp
{
  struct group g;
  cygsid sid;
};

class pwdgrp
{
  unsigned pwdgrp_buf_elem_size;
  void *pwdgrp_buf;
  bool (pwdgrp::*parse) ();
  UNICODE_STRING path;
  OBJECT_ATTRIBUTES attr;
  LARGE_INTEGER last_modified;
  char *lptr;
  ULONG curr_lines;
  ULONG max_lines;
  static muto pglock;

  bool parse_passwd ();
  bool parse_group ();
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
  void *add_account_post_fetch (char *line);
  void *add_account_from_file (cygpsid &sid);
  void *add_account_from_file (const char *name);
  void *add_account_from_file (uint32_t id);
  void *add_account_from_windows (cygpsid &sid, bool group);
  void *add_account_from_windows (const char *name, bool group);
  void *add_account_from_windows (uint32_t id, bool group);
  char *fetch_account_from_line (fetch_user_arg_t &arg, const char *line);
  char *fetch_account_from_file (fetch_user_arg_t &arg);
  char *fetch_account_from_windows (fetch_user_arg_t &arg, bool group);
  pwdgrp *prep_tls_pwbuf ();
  pwdgrp *prep_tls_grbuf ();

public:
  ULONG cached_users () const { return curr_lines; }
  ULONG cached_groups () const { return curr_lines; }
  bool check_file (bool group);

  void init_pwd ();
  pg_pwd *passwd () const { return (pg_pwd *) pwdgrp_buf; };
  inline struct passwd *add_user_from_file (cygpsid &sid)
    { return (struct passwd *) add_account_from_file (sid); }
  struct passwd *add_user_from_file (const char *name)
    { return (struct passwd *) add_account_from_file (name); }
  struct passwd *add_user_from_file (uint32_t id)
    { return (struct passwd *) add_account_from_file (id); }
  struct passwd *add_user_from_windows (cygpsid &sid)
    { return (struct passwd *) add_account_from_windows (sid, false); }
  struct passwd *add_user_from_windows (const char *name)
    { return (struct passwd *) add_account_from_windows (name, false); }
  struct passwd *add_user_from_windows (uint32_t id)
    { return (struct passwd *) add_account_from_windows (id, false); }
  struct passwd *find_user (cygpsid &sid);
  struct passwd *find_user (const char *name);
  struct passwd *find_user (uid_t uid);

  void init_grp ();
  pg_grp *group () const { return (pg_grp *) pwdgrp_buf; };
  struct group *add_group_from_file (cygpsid &sid)
    { return (struct group *) add_account_from_file (sid); }
  struct group *add_group_from_file (const char *name)
    { return (struct group *) add_account_from_file (name); }
  struct group *add_group_from_file (uint32_t id)
    { return (struct group *) add_account_from_file (id); }
  struct group *add_group_from_windows (cygpsid &sid)
    { return (struct group *) add_account_from_windows (sid, true); }
  struct group *add_group_from_windows (const char *name)
    { return (struct group *) add_account_from_windows (name, true); }
  struct group *add_group_from_windows (uint32_t id)
    { return (struct group *) add_account_from_windows (id, true); }
  struct group *find_group (cygpsid &sid);
  struct group *find_group (const char *name);
  struct group *find_group (gid_t gid);
};
