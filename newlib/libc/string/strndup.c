/*
Copyright (c) 2002 Jeff Johnston <jjohnstn@redhat.com>
 */
#ifndef _REENT_ONLY

#include <_ansi.h>
#include <stdlib.h>
#include <string.h>

char *
strndup (const char *str,
	size_t n)
{
  const char *ptr = str;
  size_t len;
  char *copy;

  while (n-- > 0 && *ptr)
    ptr++;

  len = ptr - str;

  copy = malloc (len + 1);
  if (copy)
    {
      memcpy (copy, str, len);
      copy[len] = '\0';
    }
  return copy;
}

#endif /* !_REENT_ONLY */
