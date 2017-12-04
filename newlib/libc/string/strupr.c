/*
FUNCTION
	<<strupr>>---force string to uppercase
	
INDEX
	strupr

SYNOPSIS
	#include <string.h>
	char *strupr(char *<[a]>);

DESCRIPTION
	<<strupr>> converts each character in the string at <[a]> to
	uppercase.

RETURNS
	<<strupr>> returns its argument, <[a]>.

PORTABILITY
<<strupr>> is not widely portable.

<<strupr>> requires no supporting OS subroutines.

QUICKREF
	strupr
*/

#include <string.h>
#include <ctype.h>

char *
strupr (char *s)
{
  unsigned char *ucs = (unsigned char *) s;
  for ( ; *ucs != '\0'; ucs++)
    {
      *ucs = toupper(*ucs);
    }
  return s;
}
