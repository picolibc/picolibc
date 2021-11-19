/*
Copyright (c) 2002 Jeff Johnston <jjohnstn@redhat.com>
 */
#include <wctype.h>
#include <newlib.h>
#include "check.h"

int main(void)
{
  wctrans_t x;

  x = wctrans ("tolower");
  CHECK (x != 0);
  CHECK (towctrans (L'A', x) == towlower (L'A'));
  CHECK (towctrans (L'5', x) == towlower (L'5'));

  x = wctrans ("toupper");
  CHECK (x != 0);
  CHECK (towctrans (L'c', x) == towupper (L'c'));
  CHECK (towctrans (L'9', x) == towupper (L'9'));

  x = wctrans ("unknown");
  CHECK (x == 0);

  exit (0);
}
