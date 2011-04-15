/*
FUNCTION
	<<strchrnul>>---search for character in string

INDEX
	strchrnul

ANSI_SYNOPSIS
	#include <string.h>
	char * strchrnul(const char *<[string]>, int <[c]>);

TRAD_SYNOPSIS
	#include <string.h>
	char * strchrnul(<[string]>, <[c]>);
	const char *<[string]>;
	int <[c]>;

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
_DEFUN (strchrnul, (s1, i),
	_CONST char *s1 _AND
	int i)
{
  char *s = strchr(s1, i);

  return s ? s : (char *)s1 + strlen(s1);
}
