/* textreadmode.c

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winlean.h"
#include <sys/fcntl.h>
#include <sys/cygwin.h>

extern int _fmode;
void
cygwin_premain0 (int argc __attribute__ ((unused)),
		 char **argv __attribute__ ((unused)),
		 struct per_process *myself __attribute__ ((unused)))
{
  static struct __cygwin_perfile pf[] =
    {
      {"", O_RDONLY | O_TEXT},
      {NULL, 0}
    };
  cygwin_internal (CW_PERFILE, pf);
}
