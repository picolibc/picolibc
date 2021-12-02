#include <_ansi.h>
#include <ctype.h>

#undef islower_l

int
islower_l (int c, struct __locale_t *locale)
{
  return (__locale_ctype_ptr_l (locale)[c+1] & (_U|_L)) == _L;
}
