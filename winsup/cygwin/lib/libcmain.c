/* libcmain.c

   Copyright 1996, 1997, 1998, 2000, 2001, 2004, 2006, 2009 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <windows.h>

#define SP " \t\n"

/* Allow apps which don't have a main to work, as long as they define WinMain */
int
main ()
{
  HMODULE x = GetModuleHandle (0);
  char *s = GetCommandLine ();
  STARTUPINFO si;
  char *nexts;

  s += strspn (s, SP);

  if (*s != '"')
    nexts = strpbrk (s, SP);
  else
    while ((nexts = strchr (s + 1, '"')) != NULL && nexts[-1] == '\\')
      s = nexts;

  if (!nexts)
    nexts = strchr (s, '\0');
  else if (*++nexts)
    nexts += strspn (nexts, SP);

  GetStartupInfo (&si);

  return WinMain (x, 0, nexts,
		  ((si.dwFlags & STARTF_USESHOWWINDOW) != 0
		   ? si.wShowWindow
		   : SW_SHOWNORMAL));
}
