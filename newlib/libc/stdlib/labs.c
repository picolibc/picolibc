/*
FUNCTION
<<labs>>---long integer absolute value

INDEX
	labs

SYNOPSIS
	#include <stdlib.h>
	long labs(long <[i]>);

DESCRIPTION
<<labs>> returns
@tex
$|x|$,
@end tex
the absolute value of <[i]> (also called the magnitude
of <[i]>).  That is, if <[i]> is negative, the result is the opposite
of <[i]>, but if <[i]> is nonnegative the result is <[i]>.

The similar function <<abs>> uses and returns <<int>> rather than
<<long>> values.

RETURNS
The result is a nonnegative long integer.

PORTABILITY
<<labs>> is ANSI.

No supporting OS subroutine calls are required.
*/

#include <stdlib.h>

long
labs (long x)
{
  if (x < 0)
    {
      x = -x;
    }
  return x;
}
