/*
FUNCTION
<<bzero>>---initialize memory to zero

INDEX
	bzero

ANSI_SYNOPSIS
	#include <strings.h>
	void bzero(void *<[b]>, size_t <[length]>);

TRAD_SYNOPSIS
	#include <strings.h>
	void bzero(<[b]>, <[length]>)
	void *<[b]>;
	size_t <[length]>;

DESCRIPTION
<<bzero>> initializes <[length]> bytes of memory, starting at address
<[b]>, to zero.

RETURNS
<<bzero>> does not return a result.

PORTABILITY
<<bzero>> is in the Berkeley Software Distribution.
Neither ANSI C nor the System V Interface Definition (Issue 2) require
<<bzero>>.

<<bzero>> requires no supporting OS subroutines.
*/

#include <string.h>

void
bzero(void *b, size_t length)
{

	memset(b, 0, length);
}
