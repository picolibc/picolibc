/* testsuite.cc

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* This file implements a driver for performing tests on the file/path
   translation code in path.cc.  This file is meant to be generic, all
   test harness data is in testsuite.h.  */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define TESTSUITE
#include "testsuite.h"

typedef struct
  {
    const char *cwd;    /* in win32 form, as if by GetCurrentDirectory */
    const char *posix;  /* input */
    const char *win32;  /* expected output */
  } test_t;

#define TESTSUITE_TESTS
#include "testsuite.h"
#undef TESTSUITE_TESTS

static int curtest;

/* A replacement for the w32api GetCurrentDirectory() that returns
   the cwd that the current test specifies.  */
DWORD
testsuite_getcwd (DWORD nBufferLength, LPSTR lpBuffer)
{
  unsigned len = strlen (testsuite_tests[curtest].cwd) + 1;

  /* If the test specified NO_CWD, then it means we should not have
     needed the CWD for that test as the test was for an absolute path,
     and so if we see that here return 0, simulating a
     GetCurrentDirectory() error.  */
  if (strcmp (testsuite_tests[curtest].cwd, NO_CWD) == 0)
    return 0;

  if (nBufferLength >= len)
    {
      strcpy (lpBuffer, testsuite_tests[curtest].cwd);
      return len - 1;
    }
  return len;
}

extern char *cygpath (const char *s, ...);

int
main (int argc, char **argv)
{
  int numpass = 0;

  for (test_t &t = testsuite_tests[curtest]; t.posix; t = testsuite_tests[++curtest])
    {
      char *result = cygpath (t.posix, NULL);
      bool pass = (strcmp (result, t.win32) == 0);

      if (pass)
	{
	  numpass++;
	  printf ("test %03d: PASS cwd=%-18s input=%-22s expected+actual=%s\n",
		  curtest, t.cwd, t.posix, result);
	}
      else
	{
	  printf ("test %03d: FAIL cwd=%-18s input=%-29s expected=%-25s actual=%s\n",
		  curtest, t.cwd, t.posix, t.win32, result);
	}
    }
  printf ("\n"
	  "total tests: %d\n"
	  "pass       : %d (%.1f%%)\n"
	  "fail       : %d (%.1f%%)\n",
	  curtest, numpass, ((float)numpass)/curtest * 100.0F, curtest - numpass,
	  ((float)curtest - numpass)/curtest * 100.0F);
  return (numpass < curtest ? 1 : 0);
}
