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
  bool (pwdgrp::*parse) (char *);
  int etc_ix;
  path_conv pc;
  char *buf;
  int max_lines;
  bool initialized;
  CRITICAL_SECTION lock;

  char *gets (char*&);

public:
  int curr_lines;

  bool parse_passwd (char *);
  bool parse_group (char *);
  void read_passwd ();
  void read_group ();

  void add_line (char *);
  void refresh (bool check = true)
  {
    if (initialized && check && etc::file_changed (etc_ix))
      initialized = false;
    if (!initialized)
      {
	EnterCriticalSection (&lock);
	if (!initialized)
	  (this->*read) ();
	LeaveCriticalSection (&lock);
      }
  }

  bool load (const char *);
  pwdgrp (passwd *&pbuf) :
    pwdgrp_buf_elem_size (sizeof (*pbuf)), passwd_buf (&pbuf)
    {
      read = &pwdgrp::read_passwd;
      parse = &pwdgrp::parse_passwd;
      InitializeCriticalSection (&lock);
    }
  pwdgrp (__group32 *&gbuf) :
    pwdgrp_buf_elem_size (sizeof (*gbuf)), group_buf (&gbuf)
    {
      read = &pwdgrp::read_group;
      parse = &pwdgrp::parse_group;
      InitializeCriticalSection (&lock);
    }
};
