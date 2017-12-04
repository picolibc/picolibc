/*
FUNCTION
	<<strchrnul>>---search for character in string

INDEX
	strchrnul

SYNOPSIS
	#include <string.h>
	char * strchrnul(const char *<[string]>, int <[c]>);

DESCRIPTION
	This function finds the first occurence of <[c]> (converted to
	a char) in the string pointed to by <[string]> (including the
	terminating null character).

RETURNS
	Returns a pointer to the located character, or a pointer
	to the concluding null byte if <[c]> does not occur in <[string]>.

PORTABILITY
<<strchrnul>> is a GNU extension.

<<strchrnul>> requires no supporting OS subroutines.  It uses
strchr() and strlen() from elsewhere in this library.

QUICKREF
	strchrnul
*/

#include <string.h>

char *
strchrnul (const char *s1,
	int i)
{
  char *s = strchr(s1, i);

  return s ? s : (char *)s1 + strlen(s1);
}
