/*
FUNCTION
	<<wctype_l>>---get wide-character classification type

INDEX
	wctype_l

ANSI_SYNOPSIS
	#include <wctype_l.h>
	wctype_t wctype_l(const char *<[c]>, locale_t <[locale]>);

DESCRIPTION
<<wctype_l>> is a function which takes a string <[c]> and gives back
the appropriate wctype_t type value associated with the string,
if one exists.  The following values are guaranteed to be recognized:
"alnum", "alpha", "blank", "cntrl", "digit", "graph", "lower", "print",
"punct", "space", "upper", and "xdigit".

if <[locale]> is LC_GLOBAL_LOCALE or not a valid locale object, the behaviour
is undefined.

RETURNS
<<wctype_l>> returns 0 and sets <<errno>> to <<EINVAL>> if the
given name is invalid.  Otherwise, it returns a valid non-zero wctype_t
value.

PORTABILITY
<<wctype_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/
#include <_ansi.h>
#include <wctype.h>

wctype_t
wctype_l (const char *c, struct __locale_t *locale)
{
  return wctype (c);
}
