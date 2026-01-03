/*
Copyright (c) 2016 Corinna Vinschen <corinna@vinschen.de>
Modified (m) 2017 Thomas Wolff: revise Unicode and locale/wchar handling
 */
/* Modified (m) 2017 Thomas Wolff: revise Unicode and locale/wchar handling */
#define _DEFAULT_SOURCE
#include <wctype.h>
#include <stdint.h>
#include "local.h"

wint_t
towctrans_l(wint_t u, wctrans_t w, locale_t locale)
{
    switch (w) {
    case WCT_TOLOWER:
        u = towlower_l(u, locale);
        break;
    case WCT_TOUPPER:
        u = towupper_l(u, locale);
        break;
    default:
        break;
    }
    return u;
}
