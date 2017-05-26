/* Copyright (c) 2013 Red Hat, Inc. All rights reserved.

   This copyrighted material is made available to anyone wishing to use, modify,
   copy, or redistribute it subject to the terms and conditions of the BSD
   License.   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY expressed or implied, including the implied warranties
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  A copy of this license
   is available at http://www.opensource.org/licenses. Any Red Hat trademarks that
   are incorporated in the source code or documentation are not subject to the BSD
   License and may only be used or replicated with the express permission of
   Red Hat, Inc.
*/

#include <string.h>

#include "cio.h"

static int
write_chunk (int fd, char *buf, int len)
{
  __CIOBUF__.length[0] = len;
  __CIOBUF__.length[1] = len >> 8;
  __CIOBUF__.parms[0] = CIO_WRITE;
  __CIOBUF__.parms[1] = fd;
  __CIOBUF__.parms[2] = fd >> 8;
  __CIOBUF__.parms[3] = len;
  __CIOBUF__.parms[4] = len >> 8;
  memcpy (__CIOBUF__.buf, buf, len);

  _libgloss_cio_hook ();

  return __CIOBUF__.parms[0] + __CIOBUF__.parms[1] * 256;
}

#include <stdio.h>

int
write (int fd, char *buf, int len)
{
  int rv = 0;
  int c;
#if 0
  if (fd == 2)
    fprintf (stderr, "%.*s", buf, len);
  else if (fd == 1)
    printf ("%.*s", buf, len);
#endif
  while (len > 0)
    {
      int l = (len > CIO_BUF_SIZE) ? CIO_BUF_SIZE : len;
      c = write_chunk (fd, buf, l);
      if (c < 0)
	return c;
      rv += l;
      len -= l;
      buf += l;
    }
  return rv;
}
