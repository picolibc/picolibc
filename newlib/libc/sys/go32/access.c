/* This is file ACCESS.C */
/*
** Copyright (C) 1993 DJ Delorie
**
** This file is distributed under the terms listed in the document
** "copying.dj".
** A copy of "copying.dj" should accompany this file; if not, a copy
** should be available from where this file was obtained.  This file
** may not be distributed without a verbatim copy of "copying.dj".
**
** This file is distributed WITHOUT ANY WARRANTY; without even the implied
** warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

int access(const char *fn, int flags)
{
  struct stat s;
  if (stat(fn, &s))
    return -1;
  if (s.st_mode & S_IFDIR)
    return 0;
  if (flags & W_OK)
  {
    if (s.st_mode & S_IWRITE)
      return 0;
    return -1;
  }
  return 0;
}
	
