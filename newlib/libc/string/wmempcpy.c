/*
FUNCTION
	<<wmempcpy>>---copy wide characters in memory and return end pointer

SYNOPSIS
	#define _GNU_SOURCE
	#include <wchar.h>
	wchar_t *wmempcpy(wchar_t *<[d]>,
			 const wchar_t *<[s]>, size_t <[n]>);

DESCRIPTION
	The <<wmemcpy>> function copies <[n]> wide characters from the object
	pointed to by <[s]> to the object pointed to be <[d]>. This function
	is not affected by locale and all wchar_t values are treated
	identically.  The null wide character and wchar_t values not
	corresponding to valid characters are not treated specially.

	If <[n]> is zero, <[d]> and <[s]> must be a valid pointers, and the
	function copies zero wide characters.

RETURNS
	<<wmempcpy>> returns a pointer to the wide character following the
	last wide character copied to the <[out]> region.

PORTABILITY
<<wmempcpy>> is a GNU extension.

No supporting OS subroutines are required.
*/

#define _GNU_SOURCE
#include <_ansi.h>
#include <string.h>
#include <wchar.h>

wchar_t *
wmempcpy (wchar_t *__restrict d,
	const wchar_t *__restrict s,
	size_t n)
{
  return (wchar_t *) mempcpy (d, s, n * sizeof (wchar_t));
}
