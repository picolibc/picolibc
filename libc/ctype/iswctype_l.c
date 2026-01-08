/* Copyright (c) 2016 Corinna Vinschen <corinna@vinschen.de> */
/* Modified (m) 2017 Thomas Wolff: revise Unicode and locale/wchar handling */
#define _DEFAULT_SOURCE
#include <wctype.h>
#include "local.h"

int
iswctype_l(wint_t c, wctype_t desc, locale_t locale)
{
    return __ctype_table_lookup(c, locale, desc);
}
