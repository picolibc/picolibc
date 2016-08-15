#include <_ansi.h>
#include <ctype.h>

#undef isascii_l

int
isascii_l (int c, struct __locale_t *locale)
{
  return c >= 0 && c < 128;
}
