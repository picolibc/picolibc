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
  DWORD res;
  DWORD ec = 1;
  DWORD timeout = 60 * 1000;

  if (argc < 2)
    {
      fprintf (stderr, "Usage: cygrun [program]\n");
      exit (1);
    }

  int i;
  for (i = 1; i < argc; ++i)
    {
      if (strcmp (argv[i], "-notimeout") == 0)
	timeout = INFINITE;
      else
	break;
    }

  char *command = argv[i];

  if (i < (argc-1))
    {
      fprintf (stderr, "cygrun: excess arguments\n");
      exit (1);
    }

  SetEnvironmentVariable ("CYGWIN_TESTING", "1");

  memset (&sa, 0, sizeof (sa));
  memset (&pi, 0, sizeof (pi));
  if (!CreateProcess (0, command, 0, 0, 1, 0, 0, 0, &sa, &pi))
    {
      fprintf (stderr, "CreateProcess %s failed\n", command);
      exit (1);
    }

  res = WaitForSingleObject (pi.hProcess, timeout);

  if (res == WAIT_TIMEOUT)
    {
      char cmd[1024];
      // there is no simple API to kill a Windows process tree
      sprintf(cmd, "taskkill /f /t /pid %lu", GetProcessId(pi.hProcess));
      system(cmd);
      fprintf (stderr, "Timeout\n");
      ec = 124;
    }
  else
    {
      GetExitCodeProcess (pi.hProcess, &ec);
    }

  CloseHandle (pi.hProcess);
  CloseHandle (pi.hThread);
  if (ec > 0xff)
    ec >>= 8;
  return ec;
}
