/*
FUNCTION
	<<ffs>>---find first bit set in a word

INDEX
	ffs

ANSI_SYNOPSIS
	#include <strings.h>
	int ffs(int <[word]>);

TRAD_SYNOPSIS
	#include <strings.h>
	int ffs(<[word]>);

DESCRIPTION

<<ffs>> returns the first bit set in a word.

RETURNS
<<ffs>> returns 0 if <[c]> is 0, 1 if <[c]> is odd, 2 if <[c]> is a multiple of
2, etc.

PORTABILITY
<<ffs>> is not ANSI C.

No supporting OS subroutines are required.  */

#include <strings.h>

int
ffs(int i)
{

	return (__builtin_ffs(i));
}
