/* This is file WRITEV.C */
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

#include <sys/types.h>
#include <sys/uio.h>

writev(int handle, struct iovec *iov, int count)
{
  unsigned long r, t=0;
  while (count)
  {
    r = write(handle, iov->iov_base, iov->iov_len);
    if (r < 0)
      return r;
    t += r;
    iov++;
    count--;
  }
  return t;
}
