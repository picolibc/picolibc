/*
FUNCTION
	<<strstr>>---find string segment

INDEX
	strstr

SYNOPSIS
	#include <string.h>
	char *strstr(const char *<[s1]>, const char *<[s2]>);

DESCRIPTION
	Locates the first occurrence in the string pointed to by <[s1]> of
	the sequence of characters in the string pointed to by <[s2]>
	(excluding the terminating null character).

RETURNS
	Returns a pointer to the located string segment, or a null
	pointer if the string <[s2]> is not found. If <[s2]> points to
	a string with zero length, <[s1]> is returned.

PORTABILITY
<<strstr>> is ANSI C.

<<strstr>> requires no supporting OS subroutines.

QUICKREF
	strstr ansi pure
*/

#include <string.h>

#if defined(PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__)
char *
strstr (const char *searchee,
	const char *lookfor)
{
  /* Less code size, but quadratic performance in the worst case.  */
  if (*searchee == 0)
    {
      if (*lookfor)
	return (char *) NULL;
      return (char *) searchee;
    }

  while (*searchee)
    {
      size_t i;
      i = 0;

      while (1)
	{
	  if (lookfor[i] == 0)
	    {
	      return (char *) searchee;
	    }

	  if (lookfor[i] != searchee[i])
	    {
	      break;
	    }
	  i++;
	}
      searchee++;
    }

  return (char *) NULL;

#else /* compilation for speed */

# define RETURN_TYPE char *
# define AVAILABLE(h, h_l, j, n_l)			\
  (!memchr ((h) + (h_l), '\0', (j) + (n_l) - (h_l))	\
   && ((h_l) = (j) + (n_l)))
# include "str-two-way.h"

static inline char *
strstr2 (const char *hs, const char *ne)
{
  uint32_t h1 = (ne[0] << 16) | ne[1];
  uint32_t h2 = 0;
  int c = hs[0];
  while (h1 != h2 && c != 0)
    {
      h2 = (h2 << 16) | c;
      c = *++hs;
    }
  return h1 == h2 ? (char *)hs - 2 : NULL;
}

static inline char *
strstr3 (const char *hs, const char *ne)
{
  uint32_t h1 = (ne[0] << 24) | (ne[1] << 16) | (ne[2] << 8);
  uint32_t h2 = 0;
  int c = hs[0];
  while (h1 != h2 && c != 0)
    {
      h2 = (h2 | c) << 8;
      c = *++hs;
    }
  return h1 == h2 ? (char *)hs - 3 : NULL;
}

static inline char *
strstr4 (const char *hs, const char *ne)
{
  uint32_t h1 = (ne[0] << 24) | (ne[1] << 16) | (ne[2] << 8) | ne[3];
  uint32_t h2 = 0;
  int c = hs[0];
  while (h1 != h2 && c != 0)
    {
      h2 = (h2 << 8) | c;
      c = *++hs;
    }
  return h1 == h2 ? (char *)hs - 4 : NULL;
}

char *
strstr (const char *searchee,
	const char *lookfor)
{
  /* Larger code size, but guaranteed linear performance.  */
  const char *haystack = searchee;
  const char *needle = lookfor;
  size_t needle_len; /* Length of NEEDLE.  */
  size_t haystack_len; /* Known minimum length of HAYSTACK.  */
  int ok = 1; /* True if NEEDLE is prefix of HAYSTACK.  */

  /* Handle short needle special cases first.  */
  if (needle[0] == '\0')
    return (char *) haystack;
  if (needle[1] == '\0')
    return strchr (haystack, needle[0]);
  if (needle[2] == '\0')
    return strstr2 (haystack, needle);
  if (needle[3] == '\0')
    return strstr3 (haystack, needle);
  if (needle[4] == '\0')
    return strstr4 (haystack, needle);

  /* Determine length of NEEDLE, and in the process, make sure
     HAYSTACK is at least as long (no point processing all of a long
     NEEDLE if HAYSTACK is too short).  */
  while (*haystack && *needle)
    ok &= *haystack++ == *needle++;
  if (*needle)
    return NULL;
  if (ok)
    return (char *) searchee;

  /* Reduce the size of haystack using strchr, since it has a smaller
     linear coefficient than the Two-Way algorithm.  */
  needle_len = needle - lookfor;
  haystack = strchr (searchee + 1, *lookfor);
  if (!haystack || needle_len == 1)
    return (char *) haystack;
  haystack_len = (haystack > searchee + needle_len ? 1
		  : needle_len + searchee - haystack);

  /* Perform the search.  */
  if (needle_len < LONG_NEEDLE_THRESHOLD)
    return two_way_short_needle ((const unsigned char *) haystack,
				 haystack_len,
				 (const unsigned char *) lookfor, needle_len);
  return two_way_long_needle ((const unsigned char *) haystack, haystack_len,
			      (const unsigned char *) lookfor, needle_len);
#endif /* compilation for speed */
}
