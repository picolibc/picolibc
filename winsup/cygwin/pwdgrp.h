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
extern struct passwd *internal_getpwnam (const char *, BOOL = FALSE);
extern struct passwd *internal_getpwuid (__uid32_t, BOOL = FALSE);
extern struct __group32 *internal_getgrsid (cygsid &);
extern struct __group32 *internal_getgrgid (__gid32_t gid, BOOL = FALSE);
extern struct __group32 *internal_getgrnam (const char *, BOOL = FALSE);
extern struct __group32 *internal_getgrent (int);
int internal_getgroups (int, __gid32_t *, cygsid * = NULL);

enum pwdgrp_state {
  uninitialized = 0,
  initializing,
  loaded
};

#define MAX_ETC_FILES 2
class etc
{
  static int curr_ix;
  static bool sawchange[MAX_ETC_FILES];
  static const char *fn[MAX_ETC_FILES];
  static FILETIME last_modified[MAX_ETC_FILES];
  static bool dir_changed (int);
  static int init (int, const char *);
  static bool file_changed (int);
  static void set_last_modified (int, FILETIME&);
  friend class pwdgrp;
};

class pwdgrp
{
  pwdgrp_state state;
  int pwd_ix;
  path_conv pc;
  char *buf;
  char *lptr, *eptr;

  char *gets ()
  {
    if (!buf)
      lptr = NULL;
    else if (!eptr)
      lptr = NULL;
    else
      {
	lptr = eptr;
	eptr = strchr (lptr, '\n');
	if (eptr)
	  {
	    if (eptr > lptr && *(eptr - 1) == '\r')
	      *(eptr - 1) = 0;
	    *eptr++ = '\0';
	  }
      }
    return lptr;
  }

public:
  pwdgrp () : state (uninitialized) {}
  BOOL isinitializing ()
    {
      if (state <= initializing)
	state = initializing;
      else if (etc::file_changed (pwd_ix - 1))
	state = initializing;
      return state == initializing;
    }
  void operator = (pwdgrp_state nstate) { state = nstate; }
  BOOL isuninitialized () const { return state == uninitialized; }

  bool load (const char *posix_fname, void (* add_line) (char *))
  {
    if (buf)
      free (buf);
    buf = lptr = eptr = NULL;

    pc.check (posix_fname);
    pwd_ix = etc::init (pwd_ix - 1, pc) + 1;

    paranoid_printf ("%s", posix_fname);

    bool res;
    if (pc.error || !pc.exists () || !pc.isdisk () || pc.isdir ())
      res = false;
    else
      {
	HANDLE fh = CreateFile (pc, GENERIC_READ, wincap.shared (), NULL,
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (fh == INVALID_HANDLE_VALUE)
	  res = false;
	else
	  {
	    DWORD size = GetFileSize (fh, NULL), read_bytes;
	    buf = (char *) malloc (size + 1);
	    if (!ReadFile (fh, buf, size, &read_bytes, NULL))
	      {
		if (buf)
		  free (buf);
		buf = NULL;
		fh = NULL;
		return false;
	      }
	    buf[read_bytes] = '\0';
	    eptr = buf;
	    CloseHandle (fh);
	    FILETIME ft;
	    if (GetFileTime (fh, NULL, NULL, &ft))
	      etc::set_last_modified (pwd_ix - 1, ft);
	    char *line;
	    while ((line = gets()) != NULL)
	      add_line (line);
	    res = true;
	  }
      }

    state = loaded;
    return res;
  }
};
