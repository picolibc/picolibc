/* assert.cc: Handle the assert macro for WIN32.

   Copyright 1997, 1998, 2000, 2001, 2002, 2007, 2008, 2009, 2011, 2013
   Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"

#include <assert.h>
#include <stdlib.h>

/* This function is called when the assert macro fails.  This will
   override the function of the same name in newlib.  */

extern "C" void
__assert (const char *file, int line, const char *failedexpr)
{
  __assert_func (file, line, NULL, failedexpr);
}

extern "C" void
__assert_func (const char *file, int line, const char *func,
	       const char *failedexpr)
{
  HANDLE h;

  /* If we don't have a console in a Windows program, then bring up a
     message box for the assertion failure.  */

  h = CreateFile ("CONOUT$", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
		  &sec_none_nih, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (h == INVALID_HANDLE_VALUE)
    {
      PWCHAR buf = (PWCHAR) alloca ((100 + strlen (failedexpr))
				    * sizeof (WCHAR));
      __small_swprintf (buf,
			L"Failed assertion\n\t%s\nat line %d of file %s%s%s",
			failedexpr, line, file,
			func ? "\nin function " : "", func ? func : "");
      MessageBoxW (NULL, buf, NULL, MB_OK | MB_ICONERROR | MB_TASKMODAL);
    }
  else
    {
      CloseHandle (h);
      small_printf ("assertion \"%s\" failed: file \"%s\", line %d%s%s\n",
		    failedexpr, file, line,
		    func ? ", function: " : "", func ? func : "");
      debug_printf ("assertion \"%s\" failed: file \"%s\", line %d%s%s",
		    failedexpr, file, line,
		    func ? ", function: " : "", func ? func : "");
    }

#ifdef DEBUGGING
  try_to_debug ();
#endif
  abort ();	// FIXME: Someday this should work.

  /* NOTREACHED */
}
