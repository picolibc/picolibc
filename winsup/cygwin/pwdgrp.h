/* pwdgrp.h

   Copyright 2001 Red Hat inc.

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
int internal_getgroups (int, __gid32_t *);

enum pwdgrp_state {
  uninitialized = 0,
  initializing,
  loaded
};

class pwdgrp_check {
  pwdgrp_state	state;
  FILETIME	last_modified;
  char		file_w32[MAX_PATH];

public:
  pwdgrp_check () : state (uninitialized) {}
  BOOL isinitializing ()
    {
      if (state <= initializing)
	state = initializing;
      else if (cygheap->etc_changed ())
        {
	  if (!file_w32[0])
	    state = initializing;
	  else
	    {
	      HANDLE h;
	      WIN32_FIND_DATA data;

	      if ((h = FindFirstFile (file_w32, &data)) != INVALID_HANDLE_VALUE)
	        {
		  if (CompareFileTime (&data.ftLastWriteTime, &last_modified) > 0)
		    state = initializing;
		  FindClose (h);
		}
	    }
	}
      return state == initializing;
    }
  void operator = (pwdgrp_state nstate)
    {
      state = nstate;
    }
  BOOL isuninitialized () const { return state == uninitialized; }
  void set_last_modified (HANDLE fh, const char *name)
    {
      if (!file_w32[0])
	strcpy (file_w32, name);
      GetFileTime (fh, NULL, NULL, &last_modified);
    }
};

class pwdgrp_read {
  path_conv pc;
  HANDLE fh;
  char *buf;
  char *lptr, *eptr;

public:
  bool open (const char *posix_fname)
  {
    if (buf)
      free (buf);
    buf = lptr = eptr = NULL;

    pc.check (posix_fname);
    if (pc.error || !pc.exists () || !pc.isdisk () || pc.isdir ())
      return false;

    fh = CreateFile (pc, GENERIC_READ, wincap.shared (), NULL, OPEN_EXISTING,
		     FILE_ATTRIBUTE_NORMAL, 0);
    if (fh != INVALID_HANDLE_VALUE)
      {
	DWORD size = GetFileSize (fh, NULL), read_bytes;
	buf = (char *) malloc (size + 1);
	if (!ReadFile (fh, buf, size, &read_bytes, NULL))
	  {
	    if (buf)
	      free (buf);
	    buf = NULL;
	    CloseHandle (fh);
	    fh = NULL;
	    return false;
	  }
	buf[read_bytes] = '\0';
	return true;
      }
    return false;
  }
  char *gets ()
  {
    if (!buf)
      return NULL;
    if (!lptr)
      lptr = buf;
    else if (!eptr)
      return lptr = NULL;
    else
      lptr = eptr;
    eptr = strchr (lptr, '\n');
    if (eptr)
      {
	if (eptr > lptr && *(eptr - 1) == '\r')
          *(eptr - 1) = 0;
	*eptr++ = '\0';
      }
    return lptr;
  }
  inline HANDLE get_fhandle () { return fh; }
  inline const char *get_fname () { return pc; }
  void close ()
  {
    if (fh)
      CloseHandle (fh);
    fh = NULL;
  }
};
