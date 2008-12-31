/* glob_pattern_p.c

   int glob_pattern_p (__const char *__pattern, int __quote)

   Return nonzero if PATTERN contains any metacharacters.
   Metacharacters can be quoted with backslashes if QUOTE is nonzero.

   This function is not part of the interface specified by POSIX.2
   but several programs want to use it.  */

#include <string.h>

extern "C" {

int glob_pattern_p (const char *pattern, int quote)
{
  const char *quote_chars = "\\?*[]";
  if (!quote)
    quote_chars++;
  while ((pattern = strpbrk (pattern, quote_chars)) != NULL)
    if (*pattern == '\\')
      pattern++;
    else
      return true;
  return false;
}

} /* extern "C" */
