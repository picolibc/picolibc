/* assert.cc: Handle the assert macro for WIN32.

   Copyright 1997, 1998, 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <wingdi.h>
#include <winuser.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

/* This function is called when the assert macro fails.  This will
   override the function of the same name in newlib.  */

extern "C" void
__assert (const char *file, int line, const char *failedexpr)
{
  HANDLE h;

  /* If we don't have a console in a Windows program, then bring up a
     message box for the assertion failure.  */

  h = CreateFileA ("CONOUT$", GENERIC_WRITE, FILE_SHARE_WRITE, &sec_none_nih,
		   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (h == INVALID_HANDLE_VALUE || h == 0)
    {
      char *buf;

      buf = (char *) alloca (100 + strlen (failedexpr));
      __small_sprintf (buf, "Failed assertion\n\t%s\nat line %d of file %s",
		failedexpr, line, file);
      MessageBox (NULL, buf, NULL, MB_OK | MB_ICONERROR | MB_TASKMODAL);
    }
  else
    {
      CloseHandle (h);
      small_printf ("assertion \"%s\" failed: file \"%s\", line %d\n",
		    failedexpr, file, line);
    }

  abort ();	// FIXME: Someday this should work.

  /* NOTREACHED */
}
