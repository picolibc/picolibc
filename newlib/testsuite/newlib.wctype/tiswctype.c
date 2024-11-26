/* Copyright (c) 2002 Jeff Johnston <jjohnstn@redhat.com> */
#include <wctype.h>
#include "check.h"

int main(void)
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

  CHECK(!iswprint((wint_t) __WCHAR_MAX__));
  CHECK(!iswcntrl((wint_t) __WCHAR_MAX__));
  CHECK(!iswgraph((wint_t) __WCHAR_MAX__));

  exit (0);
}
