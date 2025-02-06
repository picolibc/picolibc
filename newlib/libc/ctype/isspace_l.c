/*
Copyright (c) 2016 Corinna Vinschen <corinna@vinschen.de> 
Modified (m) 2017 Thomas Wolff: revise Unicode and locale/wchar handling
 */
#define _DEFAULT_SOURCE
#include <ctype.h>

#undef isspace_l
int
isspace_l (int c, locale_t locale)
{
    (void) locale;
#if _PICOLIBC_CTYPE_SMALL
    return isspace(c);
#else
    return __CTYPE_PTR_L (locale)[c+1] & _S;
#endif
}

