/*
FUNCTION
   <<atoi>>, <<atol>>---string to integer

INDEX
	atoi
INDEX
	atol

ANSI_SYNOPSIS
	#include <stdlib.h>
        int atoi(const char *<[s]>);
	long atol(const char *<[s]>);

TRAD_SYNOPSIS
	#include <stdlib.h>
       int atoi(<[s]>)
       char *<[s]>;

       long atol(<[s]>)
       char *<[s]>;


DESCRIPTION
   <<atoi>> converts the initial portion of a string to an <<int>>.
   <<atol>> converts the initial portion of a string to a <<long>>.

   <<atoi(s)>> is implemented as <<(int)strtol(s, NULL, 10).>>
   <<atol(s)>> is implemented as <<strtol(s, NULL, 10).>>

RETURNS
   The functions return the converted value, if any. If no conversion was
   made, <<0>> is returned.

PORTABILITY
<<atoi>> is ANSI.

No supporting OS subroutines are required.
*/

/*
 * Andy Wilson, 2-Oct-89.
 */

#include <stdlib.h>
#include <_ansi.h>

int
_DEFUN (atoi, (s),
	_CONST char *s)
{
  return (int) strtol (s, NULL, 10);
}

