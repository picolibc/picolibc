/*
FUNCTION
	<<strlwr>>---force string to lowercase
	
INDEX
	strlwr

ANSI_SYNOPSIS
	#include <string.h>
	char *strlwr(char *<[a]>);

TRAD_SYNOPSIS
	#include <string.h>
	char *strlwr(<[a]>)
	char *<[a]>;

DESCRIPTION
	<<strlwr>> converts each character in the string at <[a]> to
	lowercase.

RETURNS
	<<strlwr>> returns its argument, <[a]>.

PORTABILITY
<<strlwr>> is not widely portable.

<<strlwr>> requires no supporting OS subroutines.

QUICKREF
	strlwr
*/

#include <string.h>
#include <ctype.h>

char *
_DEFUN (strlwr, (s),
	char *s)
{
  unsigned char *ucs = (unsigned char *) s;
  for ( ; *ucs != '\0'; ucs++)
    {
      *ucs = tolower(*ucs);
    }
  return s;
}
