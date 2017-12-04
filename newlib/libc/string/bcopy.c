/*
FUNCTION
	<<bcopy>>---copy memory regions

SYNOPSIS
	#include <strings.h>
	void bcopy(const void *<[in]>, void *<[out]>, size_t <[n]>);

DESCRIPTION
	This function copies <[n]> bytes from the memory region
	pointed to by <[in]> to the memory region pointed to by
	<[out]>.

	This function is implemented in term of <<memmove>>.

PORTABILITY
<<bcopy>> requires no supporting OS subroutines.

QUICKREF
	bcopy - pure
*/

#include <string.h>
#include <strings.h>

void
bcopy (const void *b1,
	void *b2,
	size_t length)
{
  memmove (b2, b1, length);
}
