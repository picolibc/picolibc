/*
FUNCTION
	<<rindex>>---reverse search for character in string

INDEX
	rindex

SYNOPSIS
	#include <string.h>
	char * rindex(const char *<[string]>, int <[c]>);

DESCRIPTION
	This function finds the last occurence of <[c]> (converted to
	a char) in the string pointed to by <[string]> (including the
	terminating null character).

	This function is identical to <<strrchr>>.

RETURNS
	Returns a pointer to the located character, or a null pointer
	if <[c]> does not occur in <[string]>.

PORTABILITY
<<rindex>> requires no supporting OS subroutines.

QUICKREF
	rindex - pure
*/

#include <string.h>
#include <strings.h>

char *
rindex (const char *s,
	int c)
{
  return strrchr (s, c);
}
