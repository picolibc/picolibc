#undef __STRICT_ANSI__
#include <_ansi.h>
#include <string.h>

/*
 * Find the first occurrence of find in s, where the search is limited to the
 * first slen characters of s.
 */
char *
strnstr(const char *haystack, const char *needle, size_t haystack_len)
{
  size_t needle_len = strnlen(needle, haystack_len);

  if (needle_len < haystack_len || !needle[needle_len]) {
    char *x = memmem(haystack, haystack_len, needle, needle_len);
    if (x && !memchr(haystack, 0, x - haystack))
      return x;
  }
  return NULL;
}
