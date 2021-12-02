#include <_ansi.h>
#include <ctype.h>

#undef isblank_l

int
isblank_l (int c, struct __locale_t *locale)
{
  return (__locale_ctype_ptr_l (locale)[c+1] & _B) || (c == '\t');
}
