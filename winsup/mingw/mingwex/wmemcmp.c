/*	This source code was extracted from the Q8 package created and placed
    in the PUBLIC DOMAIN by Doug Gwyn <gwyn@arl.mil>
    last edit:	1999/11/05	gwyn@arl.mil

    Implements subclause 7.24 of ISO/IEC 9899:1999 (E).

	It supports an encoding where all char codes are mapped
	to the *same* code values within a wchar_t or wint_t,
	so long as no other wchar_t codes are used by the program.

*/

#include	<wchar.h>

int
wmemcmp(s1, s2, n)
	register const wchar_t	*s1;
	register const wchar_t	*s2;
	size_t				n;
	{
	if ( n == 0 || s1 == s2 )
		return 0;		/* even for NULL pointers */

	if ( (s1 != NULL) != (s2 != NULL) )
		return s2 == NULL ? 1 : -1;	/* robust */

	for ( ; n > 0; ++s1, ++s2, --n )
		if ( *s1 != *s2 )
			return *s1 - *s2;

	return 0;
	}
