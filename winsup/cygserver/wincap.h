/* wincap.h: Header for OS capability class.
	     Lightweight version for Cygserver.

   Copyright 2006 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGSERVER_WINCAP_H
#define _CYGSERVER_WINCAP_H

class wincapc
{
  OSVERSIONINFO version;

public:
  void init ();

  bool is_winnt () const
    { return version.dwPlatformId == VER_PLATFORM_WIN32_NT; }
  bool has_security () const
    { return version.dwPlatformId == VER_PLATFORM_WIN32_NT; }
};

extern wincapc wincap;

#endif /* _CYGSERVER_WINCAP_H */
