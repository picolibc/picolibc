/* link stub for MMIXware.

   Copyright (C) 2002, 2023 Hans-Peter Nilsson

   Permission to use, copy, modify, and distribute this software is
   freely granted, provided that the above copyright notice, this notice
   and the following disclaimer are preserved with no changes.

   THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   PURPOSE.  */

#include <_ansi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "sys/syscall.h"
#include <errno.h>

int
_link (const char *p1 __unused, const char *p2 __unused)
{
  errno = EMLINK;
  return -1;
}
