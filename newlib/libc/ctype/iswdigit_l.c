#include <_ansi.h>
#include <wctype.h>

int
iswdigit_l (wint_t c, struct __locale_t *locale)
{
  return (c >= (wint_t)'0' && c <= (wint_t)'9');
}
