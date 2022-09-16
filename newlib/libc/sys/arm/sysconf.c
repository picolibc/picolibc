/* libc/sys/arm/sysconf.c - The sysconf function */

/* Copyright 2020, STMicroelectronics
 *
 * All rights reserved.
 *
 * Redistribution, modification, and use in source and binary forms is permitted
 * provided that the above copyright notice and following paragraph are
 * duplicated in all such forms.
 *
 * This file is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <unistd.h>
#include <errno.h>

long sysconf(int name)
{
  switch (name)
  {
  case _SC_PAGESIZE:
#ifdef SMALL_MEMORY
    return 128;
#else
    return 4096;
#endif

  default:
    errno = EINVAL;
    return -1;
  }
  return -1; /* Can't get here */
}
