/*	This source code was extracted from the Q8 package created and placed
    in the PUBLIC DOMAIN by Doug Gwyn <gwyn@arl.mil>
    last edit:	1999/11/05	gwyn@arl.mil

    Implements subclause 7.24 of ISO/IEC 9899:1999 (E).

	It supports an encoding where all char codes are mapped
	to the *same* code values within a wchar_t or wint_t,
	so long as no other wchar_t codes are used by the program.

*/

#include	<wchar.h>

wchar_t*
wmemchr(s, c, n)
	register const wchar_t	*s;
	register wchar_t		c;
	register size_t			n;
	{
	if ( s != NULL )
		for ( ; n > 0; ++s, --n )
			if ( *s == c )
				return (wchar_t *)s;

	return NULL;
	}

