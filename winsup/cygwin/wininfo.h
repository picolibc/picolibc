/* wininfo.h: main Cygwin header file.

   Copyright 2004 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

class muto;
class wininfo
{
  HWND hwnd;
  static muto *lock;
public:
  UINT timer_active;
  struct itimerval itv;
  DWORD start_time;
  HANDLE window_started;
  operator HWND ();
  int __stdcall wininfo::process (HWND, UINT, WPARAM, LPARAM)
    __attribute__ ((regparm (3)));
  int __stdcall setitimer (const struct itimerval *value, struct itimerval *oldvalue)
    __attribute__ ((regparm (3)));
  int __stdcall getitimer (struct itimerval *value)
    __attribute__ ((regparm (2)));
  wininfo ();
  DWORD WINAPI winthread () __attribute__ ((regparm (1)));
};

extern wininfo winmsg;
