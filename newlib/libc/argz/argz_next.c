/* Copyright (C) 2002 by  Red Hat, Incorporated. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

#include <argz.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

char *
argz_next (char *argz,
       size_t argz_len,
       const char *entry)
{
  if (entry)
    {
      while(*entry != '\0')
        entry++;
      entry++;

      if (entry >= argz + argz_len)
        return NULL;
      else
        return (char *) entry;
    }
  else
    {
      if (argz_len > 0)
        return (char *) argz;
      else
        return NULL;
    }
}
