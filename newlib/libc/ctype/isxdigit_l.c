/*
Copyright (c) 2016 Corinna Vinschen <corinna@vinschen.de> 
Modified (m) 2017 Thomas Wolff: revise Unicode and locale/wchar handling
 */

#define _DEFAULT_SOURCE
#include <ctype.h>

#undef isxdigit_l
int
isxdigit_l (int c, locale_t locale)
{
    (void) locale;
#if _PICOLIBC_CTYPE_SMALL
    return isxdigit(c);
#else
    return __locale_ctype_ptr_l (locale)[c+1] & ((_X)|(_N));
#endif
}

