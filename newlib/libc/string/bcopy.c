/*
FUNCTION
	<<bcopy>>---copy memory regions

ANSI_SYNOPSIS
	#include <string.h>
	void bcopy(const char *<[in]>, char  *<[out]>, size_t <[n]>);

TRAD_SYNOPSIS
	void bcopy(<[in]>, <[out]>, <[n]>
	char *<[in]>;
	char *<[out]>;
	size_t <[n]>;

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

void
_DEFUN (bcopy, (b1, b2, length),
	_CONST char *b1 _AND
	char *b2 _AND
	size_t length)
{
  memmove ((_PTR) b2, (_PTR) b1, length);
}
