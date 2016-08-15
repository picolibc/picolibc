#include <_ansi.h>
#include <ctype.h>

#undef toascii_l

int
toascii_l (int c, struct __locale_t *locale)
{
  return c & 0177;
}
