/* pwdgrp.h

   Copyright 2001 Red Hat inc.

   Stuff common to pwd and grp handling.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

enum pwdgrp_state {
  uninitialized = 0,
  initializing,
  emulated,
  loaded
};

class pwdgrp_check {
  pwdgrp_state	state;
  FILETIME	last_modified;
  char		file_w32[MAX_PATH];

public:
  pwdgrp_check () : state (uninitialized) {}
  operator pwdgrp_state ()
    {
      if (state != uninitialized && file_w32[0] && cygheap->etc_changed ())
        {
          HANDLE h;
          WIN32_FIND_DATA data;

          if ((h = FindFirstFile (file_w32, &data)) != INVALID_HANDLE_VALUE)
            {
              if (CompareFileTime (&data.ftLastWriteTime, &last_modified) > 0)
                state = uninitialized;
              FindClose (h);
            }
        }
      return state;
    }
  void operator = (pwdgrp_state nstate)
    {
      state = nstate;
    }
  void set_last_modified (FILE *f)
    {
      if (!file_w32[0])
        strcpy (file_w32, cygheap->fdtab[fileno (f)]->get_win32_name ());

      GetFileTime (cygheap->fdtab[fileno (f)]->get_handle (),
		   NULL, NULL, &last_modified);
    }
};
