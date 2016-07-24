/*
FUNCTION
	<<strxfrm_l>>---transform string

INDEX
	strxfrm_l

ANSI_SYNOPSIS
	#include <string.h>
	size_t strxfrm_l(char *restrict <[s1]>, const char *restrict <[s2]>,
                       size_t <[n]>, locale_t <[locale]>);

DESCRIPTION
	This function transforms the string pointed to by <[s2]> and
	places the resulting string into the array pointed to by
	<[s1]>. The transformation is such that if the <<strcmp>>
	function is applied to the two transformed strings, it returns
	a value greater than, equal to, or less than zero,
	correspoinding to the result of a <<strcoll>> function applied
	to the same two original strings.

	No more than <[n]> characters are placed into the resulting
	array pointed to by <[s1]>, including the terminating null
	character. If <[n]> is zero, <[s1]> may be a null pointer. If
	copying takes place between objects that overlap, the behavior
	is undefined.

	(NOT Cygwin:) The current implementation of <<strxfrm_l>> simply copies
	the input and does not support any language-specific transformations.

	If <[locale]> is LC_GLOBAL_LOCALE or not a valid locale object, the
	behaviour is undefined.

RETURNS
	The <<strxfrm_l>> function returns the length of the transformed string
	(not including the terminating null character). If the value returned
	is <[n]> or more, the contents of the array pointed to by
	<[s1]> are indeterminate.

PORTABILITY
<<strxfrm_l>> is POSIX-1.2008.

<<strxfrm_l>> requires no supporting OS subroutines.

QUICKREF
	strxfrm_l ansi pure
*/

#include <string.h>

size_t
strxfrm_l (char *__restrict s1, const char *__restrict s2, size_t n,
	   struct __locale_t *locale)
{
  size_t res;
  res = 0;
  while (n-- > 0)
    {
      if ((*s1++ = *s2++) != '\0')
        ++res;
      else
        return res;
    }
  while (*s2)
    {
      ++s2;
      ++res;
    }

  return res;
}
