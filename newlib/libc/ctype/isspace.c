/*
FUNCTION
	<<isspace>>, <<isspace_l>>---whitespace character predicate

INDEX
	isspace

INDEX
	isspace_l

SYNOPSIS
	#include <ctype.h>
	int isspace(int <[c]>);

	#include <ctype.h>
	int isspace_l(int <[c]>, locale_t <[locale]>);

DESCRIPTION
<<isspace>> is a macro which classifies singlebyte charset values by table
lookup.  It is a predicate returning non-zero for whitespace
characters, and 0 for other characters.  It is defined only when <<isascii>>(<[c]>) is true or <[c]> is EOF.

<<isspace_l>> is like <<isspace>> but performs the check based on the
locale specified by the locale object locale.  If <[locale]> is
LC_GLOBAL_LOCALE or not a valid locale object, the behaviour is undefined.

You can use a compiled subroutine instead of the macro definition by
undefining the macro using `<<#undef isspace>>' or `<<#undef isspace_l>>'.

RETURNS
<<isspace>>, <<isspace_l>> return non-zero if <[c]> is a space, tab,
carriage return, new line, vertical tab, or formfeed (<<0x09>>--<<0x0D>>,
<<0x20>>), or one of the other space characters in non-ASCII charsets.

PORTABILITY
<<isspace>> is ANSI C.
<<isspace_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/
#include <_ansi.h>
#include <ctype.h>


#undef isspace
int
isspace (int c)
{
	return(__CTYPE_PTR[c+1] & _S);
}
