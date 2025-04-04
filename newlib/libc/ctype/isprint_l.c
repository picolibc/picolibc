/*
Copyright (c) 2016 Corinna Vinschen <corinna@vinschen.de> 
Modified (m) 2017 Thomas Wolff: revise Unicode and locale/wchar handling
 */

#define _DEFAULT_SOURCE
#include <ctype.h>

#undef isprint_l
int
isprint_l (int c, locale_t locale)
{
#if _PICOLIBC_CTYPE_SMALL
    (void) locale;
    return isprint(c);
#else
    return __CTYPE_PTR_L (locale)[c+1] & (__CTYPE_PUNCT|__CTYPE_UPPER|__CTYPE_LOWER|__CTYPE_DIGIT|__CTYPE_BLANK);
#endif
}
