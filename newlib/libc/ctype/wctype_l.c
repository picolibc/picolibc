#include <_ansi.h>
#include <wctype.h>

wctype_t
wctype_l (const char *c, struct __locale_t *locale)
{
  return wctype (c);
}
