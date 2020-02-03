/*
Copyright (c) 2002 Jeff Johnston <jjohnstn@redhat.com>
 */
#ifndef _REENT_ONLY

#include <stdlib.h>
#include <string.h>

char *
strdup (const char *str)
{
  size_t len = strlen (str) + 1;
  char *copy = malloc (len);
  if (copy)
    {
      memcpy (copy, str, len);
    }
  return copy;
}

#endif /* !_REENT_ONLY */
