#define __NO_CTYPE_LINES
#include <wctype.h>

int __cdecl iswblank (wint_t wc)
  {return (iswctype(wc, _BLANK) || wc == L'\t');}
