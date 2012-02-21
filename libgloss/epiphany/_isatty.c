/* isatty.c

   Copyright (c) 1994-2009  Red Hat, Inc. All rights reserved.
   Copyright (c) 2012 Adapteva, Inc.

   This copyrighted material is made available to anyone wishing to use,
   modify, copy, or redistribute it subject to the terms and conditions
   of the BSD License.   This program is distributed in the hope that
   it will be useful, but WITHOUT ANY WARRANTY expressed or implied,
   including the implied warranties of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  A copy of this license is available at
   http://www.opensource.org/licenses. Any Red Hat trademarks that are
   incorporated in the source code or documentation are not subject to
   the BSD License and may only be used or replicated with the express
   permission of Red Hat, Inc.  */

/* Dumb implementation so programs will at least run.  */

#include <sys/stat.h>
#include <errno.h>

int
_DEFUN(_isatty, (fd), int fd)
{
  struct stat buf;

  if (_fstat (fd, &buf) < 0) {
    errno = EBADF;
    return 0;
  }
  if (S_ISCHR (buf.st_mode))
    return 1;
  errno = ENOTTY;
  return 0;
}
