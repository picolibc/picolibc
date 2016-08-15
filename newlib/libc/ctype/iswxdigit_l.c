#include <_ansi.h>
#include <wctype.h>

int
iswxdigit_l (wint_t c, struct __locale_t *locale)
{
  return ((c >= (wint_t)'0' && c <= (wint_t)'9') ||
	  (c >= (wint_t)'a' && c <= (wint_t)'f') ||
	  (c >= (wint_t)'A' && c <= (wint_t)'F'));
}

