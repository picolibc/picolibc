/*
FUNCTION
	<<index>>---search for character in string

INDEX
	index

SYNOPSIS
	#include <strings.h>
	char * index(const char *<[string]>, int <[c]>);

DESCRIPTION
	This function finds the first occurence of <[c]> (converted to
	a char) in the string pointed to by <[string]> (including the
	terminating null character).

	This function is identical to <<strchr>>.

RETURNS
	Returns a pointer to the located character, or a null pointer
	if <[c]> does not occur in <[string]>.

PORTABILITY
<<index>> requires no supporting OS subroutines.

QUICKREF
	index - pure
*/

#include <string.h>
#include <strings.h>

char *
index (const char *s,
	int c)
{
  return strchr (s, c);
}
