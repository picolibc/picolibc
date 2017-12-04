/* Copyright (C) 2002 by  Red Hat, Incorporated. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

#include <_ansi.h>
#include <argz.h>
#include <stddef.h>
#include <sys/types.h>

size_t
argz_count (const char *argz,
       size_t argz_len)
{
  int i;
  size_t count = 0;

  for (i = 0; i < argz_len; i++)
    {
      if (argz[i] == '\0')
        count++;
    }
  return count;
}
