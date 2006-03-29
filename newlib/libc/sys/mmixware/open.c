/* open for MMIXware.

   Copyright (C) 2001 Hans-Peter Nilsson

   Permission to use, copy, modify, and distribute this software is
   freely granted, provided that the above copyright notice, this notice
   and the following disclaimer are preserved with no changes.

   THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   PURPOSE.  */

#include <fcntl.h>
#include <_ansi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "sys/syscall.h"
#include <errno.h>

/* Let's keep the filehandle array here, since this is a primary
   initializer of it.  */
unsigned char _MMIX_allocated_filehandle[32] =
 {
   1,
   1,
   1,
   0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0
 };

int
_open (const char *path,
       int flags, ...)
{
  long fileno;
  unsigned char mode;
  long fffile = 0;
  long ret;

  for (fileno = 0;
       fileno < (sizeof (_MMIX_allocated_filehandle) /
		 sizeof (_MMIX_allocated_filehandle[0]));
       fileno++)
    if (_MMIX_allocated_filehandle[fileno] == 0)
      break;

  if (fileno == (sizeof (_MMIX_allocated_filehandle) /
		 sizeof (_MMIX_allocated_filehandle[0])))
    {
      errno = EMFILE;
      return -1;
    }

  /* We map this to a fopen call.  The flags parameter is stymied because
     we don't support more other than these flags.  */
  if (flags & ~(O_RDONLY | O_WRONLY | O_RDWR | O_CREAT | O_APPEND | O_TRUNC))
    {
      UNIMPLEMENTED (("path: %s, flags: %d", path, flags));
      errno = ENOSYS;
      return -1;
    }

  if ((flags & O_ACCMODE) == O_RDONLY)
    mode = BinaryRead;
  else if ((flags & (O_WRONLY | O_APPEND)) == (O_WRONLY | O_APPEND))
    {
      mode = BinaryReadWrite;
      fffile = 1;
    }
  else if ((flags & (O_RDWR | O_APPEND)) == (O_RDWR | O_APPEND))
    {
      mode = BinaryReadWrite;
      fffile = 1;
    }
  else if ((flags & (O_WRONLY | O_CREAT)) == (O_WRONLY | O_CREAT)
	   || (flags & (O_WRONLY | O_TRUNC)) == (O_WRONLY | O_TRUNC))
    mode = BinaryWrite;
  else if ((flags & (O_RDWR | O_CREAT)) == (O_RDWR | O_CREAT))
    mode = BinaryReadWrite;
  else if (flags & O_RDWR)
    mode = BinaryReadWrite;
  else
    {
      errno = EINVAL;
      return -1;
    }

  ret = TRAP3f (SYS_Fopen, fileno, path, mode);
  if (ret < 0)
    {
      /* It's totally unknown what the error was.  We'll just take our
	 chances and assume ENOENT.  */
      errno = ENOENT;
      return -1;
    }

  _MMIX_allocated_filehandle[fileno] = 1;

  if (fffile)
    {
      TRAP2f (SYS_Fseek, fileno, -1);
    }

  return fileno;
}
