#include <_ansi.h>
#include <ctype.h>

#undef isgraph_l

int
isgraph_l (int c, struct __locale_t *locale)
{
  return __locale_ctype_ptr_l (locale)[c+1] & (_P|_U|_L|_N);
}

#undef isprint_l

int
isprint_l (int c, struct __locale_t *locale)
{
  return __locale_ctype_ptr_l (locale)[c+1] & (_P|_U|_L|_N|_B);
}
