#include <wctype.h>
#include <newlib.h>
#include "check.h"

int main()
{
  wctype_t x;

  x = wctype ("alpha");
  CHECK (x != 0);
  CHECK (iswctype (L'a', x) && iswalpha (L'a'));

  x = wctype ("alnum");
  CHECK (x != 0);
  CHECK (iswctype (L'0', x) && iswalnum (L'0'));

  x = wctype ("blank");
  CHECK (x != 0);
  CHECK (iswctype (L' ', x) && iswblank (L' '));

  x = wctype ("cntrl");
  CHECK (x != 0);
  CHECK (iswctype (L'\n', x) && iswcntrl (L'\n'));

  x = wctype ("digit");
  CHECK (x != 0);
  CHECK (iswctype (L'7', x) && iswdigit (L'7'));

  x = wctype ("graph");
  CHECK (x != 0);
  CHECK (iswctype (L'!', x) && iswgraph (L'!'));

  x = wctype ("lower");
  CHECK (x != 0);
  CHECK (iswctype (L'k', x) && iswlower (L'k'));

  x = wctype ("print");
  CHECK (x != 0);
  CHECK (iswctype (L'@', x) && iswprint (L'@'));

  x = wctype ("punct");
  CHECK (x != 0);
  CHECK (iswctype (L'.', x) && iswpunct (L'.'));

  x = wctype ("space");
  CHECK (x != 0);
  CHECK (iswctype (L'\t', x) && iswspace (L'\t'));

  x = wctype ("upper");
  CHECK (x != 0);
  CHECK (iswctype (L'T', x) && iswupper (L'T'));

  x = wctype ("xdigit");
  CHECK (x != 0);
  CHECK (iswctype (L'B', x) && iswxdigit (L'B'));

  x = wctype ("unknown");
  CHECK (x == 0);

  exit (0);
}
