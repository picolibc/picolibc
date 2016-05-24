/* string.h: Extra string defs

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGWIN_STRING_H
#define _CYGWIN_STRING_H

#include_next <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#undef strchrnul
#define strchrnul cygwin_strchrnul
static inline __stdcall char *
strchrnul (const char *s, int c)
{
  while (*s != (char) c && *s != 0)
    ++s;
  return (char *) s;
}

#ifdef __INSIDE_CYGWIN__

extern const char isalpha_array[];

static inline int
ascii_strcasematch (const char *cs, const char *ct)
{
  register const unsigned char *us, *ut;

  us = (const unsigned char *) cs;
  ut = (const unsigned char *) ct;

  while (us[0] == ut[0] || (us[0] ^ isalpha_array[us[0]]) == ut[0])
    {
      if (us[0] == 0)
	return 1;
      ++us, ++ut;
    }
  return 0;
}

static inline int
ascii_strncasematch (const char *cs, const char *ct, size_t n)
{
  register const unsigned char *us, *ut;

  if (!n)
   return 1;
  us = (const unsigned char *) cs;
  ut = (const unsigned char *) ct;

  while (us[0] == ut[0] || (us[0] ^ isalpha_array[us[0]]) == ut[0])
    {
      --n;
      if (!n || us[0] == 0)
        return 1;
      ++us, ++ut;
    }
  return 0;
}

#undef strcasecmp
#define strcasecmp cygwin_strcasecmp
int __stdcall cygwin_strcasecmp (const char *, const char *);

#undef strncasecmp
#define strncasecmp cygwin_strncasecmp
int __stdcall cygwin_strncasecmp (const char *, const char *, size_t);

#define strcasematch(s1,s2)	(!cygwin_strcasecmp ((s1),(s2)))
#define strncasematch(s1,s2,n)	(!cygwin_strncasecmp ((s1),(s2),(n)))

char *strlwr (char *);
char *strupr (char *);

#endif /* __INSIDE_CYGWIN__ */

char *__stdcall strccpy (char *__restrict s1, const char **__restrict s2,
			 char c);

#ifdef __cplusplus
}
#endif

#endif /* _CYGWIN_STRING_H */
