/* cygrun.c: testsuite support program

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* This program is intended to be used only by the testsuite.  It runs
   programs without using the cygwin api, so that the just-built dll
   can be tested without interference from the currently installed
   dll. */

#include <stdio.h>
#include <windows.h>
#include <stdlib.h>

int
main (int argc, char **argv)
{
  STARTUPINFO sa;
  PROCESS_INFORMATION pi;
  DWORD ec = 1;
  char *p;

  if (argc < 2)
    {
      fprintf (stderr, "Usage: cygrun [program] [tmpdir]\n");
      exit (0);
    }

  if (argc >= 3)
    SetEnvironmentVariable ("TDIRECTORY", argv[2]);

  SetEnvironmentVariable ("CYGWIN_TESTING", "1");

  memset (&sa, 0, sizeof (sa));
  memset (&pi, 0, sizeof (pi));
  if (!CreateProcess (0, argv[1], 0, 0, 1, 0, 0, 0, &sa, &pi))
    {
      fprintf (stderr, "CreateProcess %s failed\n", argv[1]);
      exit (1);
    }

  WaitForSingleObject (pi.hProcess, INFINITE);

  GetExitCodeProcess (pi.hProcess, &ec);

  CloseHandle (pi.hProcess);
  CloseHandle (pi.hThread);
  if (ec > 0xff)
    ec >>= 8;
  return ec;
}
