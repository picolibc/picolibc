#include <string.h>

#include "cio.h"

signed int
unlink (const char * name)
{
  unsigned len = strlen (name);

  if (len >= CIO_BUF_SIZE)
    return -1;

  __CIOBUF__.length[0] = len;
  __CIOBUF__.length[1] = len >> 8;
  __CIOBUF__.parms[0] = CIO_UNLINK;
  __CIOBUF__.parms[1] = len;
  __CIOBUF__.parms[2] = len >> 8;
  memcpy (__CIOBUF__.buf, name, len);

  _libgloss_cio_hook ();

  return __CIOBUF__.parms[0] + __CIOBUF__.parms[1] * 256;
}


