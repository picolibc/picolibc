/* access for MMIXware.

   Copyright (C) 2001 Hans-Peter Nilsson

   Permission to use, copy, modify, and distribute this software is
   freely granted, provided that the above copyright notice, this notice
   and the following disclaimer are preserved with no changes.

   THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   PURPOSE.  */

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include "sys/syscall.h"
#include <errno.h>

int
access (const char *fn, int flags)
{
  /* Ugh.  We don't have stat (), so we can't copy the other
     implementations.  Opening a directory as a file usually works, so
     let's try and open it and use the openability, regardless of what
     kind of test or file it is.  */
  long ret;

  /* We'll just assume that if we can open the file for reading, then it's
     Z-able, no matter what Z was.  */
  ret = TRAP3f (SYS_Fopen, TMPFNO, fn, BinaryRead);
  if (ret == 0)
    {
      /* Yes, this was readable.  As in other simulator access functions,
	 we always return success in this case (though the others check
	 for directory access).  */
      TRAP1f (SYS_Fclose, TMPFNO);
      return 0;
    }

  errno = EACCES;
  return -1;
}
