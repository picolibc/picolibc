/* libcmain.c

   Copyright 1996, 1997, 1998, 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <windows.h>

/* Allow apps which don't have a main work, as long as they define WinMain */
int
main ()
{
  HMODULE x = GetModuleHandleA(0);
  char *s = GetCommandLineA ();
  STARTUPINFO si;

  /* GetCommandLineA returns the entire command line including the
     program name, but WinMain is defined to accept the command
     line without the program name.  */
  while (*s != ' ' && *s != '\0')
    ++s;
  while (*s == ' ')
    ++s;

  GetStartupInfo (&si);

  return WinMain (x, 0, s,
		  ((si.dwFlags & STARTF_USESHOWWINDOW) != 0
		   ? si.wShowWindow
		   : SW_SHOWNORMAL));
}
