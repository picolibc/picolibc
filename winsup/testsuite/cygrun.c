/* cygrun.c: testsuite support program

   Copyright 1999, 2000, 2001, 2002 Red Hat, Inc.

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
      fprintf (stderr, "Usage: cygrun [program]\n");
      exit (0);
    }

  SetEnvironmentVariable ("CYGWIN_TESTING", "1");
  if ((p = getenv ("CYGWIN")) == NULL || (strstr (p, "ntsec") == NULL))
    {
      char buf[4096];
      if (!p)
	{
	  p = buf;
	  p[0] = '\0';
	}
      else
	{
	  strcpy (buf, p);
	  strcat (buf, " ");
	}
      strcat(buf, "ntsec");
      SetEnvironmentVariable ("CYGWIN", buf);
    }

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
