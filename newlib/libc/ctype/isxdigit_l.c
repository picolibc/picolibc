/*
Copyright (c) 2016 Corinna Vinschen <corinna@vinschen.de> 
Modified (m) 2017 Thomas Wolff: revise Unicode and locale/wchar handling
 */

#define _DEFAULT_SOURCE
#include <ctype.h>

#undef isxdigit_l
int
isxdigit_l (int c, struct __locale_t *locale)
{
#ifdef __HAVE_LOCALE_INFO__
    return __locale_ctype_ptr_l (locale)[c+1] & ((_X)|(_N));
#else
    (void) locale;
    return isxdigit(c);
#endif
}

