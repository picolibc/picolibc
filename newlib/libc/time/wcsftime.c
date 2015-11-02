/*
 * wcsftime.c
 * Original Author:	Craig Howland, for Newlib
 *
 * Source actually uses strftime.c.
 * Documentation for wcsftime() here, with minimal overlap.
 */

/*
FUNCTION
<<wcsftime>>---convert date and time to a formatted wide-character string

INDEX
	wcsftime

ANSI_SYNOPSIS
	#include <time.h>
	#include <wchar.h>
	size_t wcsftime(wchar_t *<[s]>, size_t <[maxsize]>,
			const wchar_t *<[format]>, const struct tm *<[timp]>);

DESCRIPTION
<<wcsftime>> is equivalent to <<strftime>>, except that:
 
O+
o The argument s points to the initial element of an array of wide characters
into which the generated output is to be placed.
 
o The argument maxsize indicates the limiting number of wide characters.
 
o The argument format is a wide-character string and the conversion specifiers
are replaced by corresponding sequences of wide characters.
 
o The return value indicates the number of wide characters.
O-
(The difference in all of the above being wide characters versus regular
characters.)
 
See <<strftime>> for the details of the format specifiers.

RETURNS
When the formatted time takes up no more than <[maxsize]> wide characters,
the result is the length of the formatted wide string.  Otherwise, if the
formatting operation was abandoned due to lack of room, the result is
<<0>>, and the wide-character string starting at <[s]> corresponds to just those
parts of <<*<[format]>>> that could be completely filled in within the
<[maxsize]> limit.

PORTABILITY
C99 and POSIX require <<wcsftime>>, but do not specify the contents of
<<*<[s]>>> when the formatted string would require more than
<[maxsize]> characters.  Unrecognized specifiers and fields of
<<timp>> that are out of range cause undefined results.  Since some
formats expand to 0 bytes, it is wise to set <<*<[s]>>> to a nonzero
value beforehand to distinguish between failure and an empty string.
This implementation does not support <<s>> being NULL, nor overlapping
<<s>> and <<format>>.

<<wcsftime>> requires no supporting OS subroutines.

SEEALSO
<<strftime>>
*/

#include <time.h>
#include <wchar.h>
#define MAKE_WCSFTIME
#include "../time/strftime.c"
