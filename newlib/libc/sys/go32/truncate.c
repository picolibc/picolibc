/* This is file TRUNCATE.C */
/*
** Copyright (C) 1991 DJ Delorie
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

truncate(const char *fn, unsigned long where)
{
  int fd = open(fn, O_WRONLY);
  if (fd < 0)
    return -1;
  lseek(fd, where, 0);
  write(fd, 0, 0);
  close(fd);
}
