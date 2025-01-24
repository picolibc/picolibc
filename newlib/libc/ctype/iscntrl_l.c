/*
Copyright (c) 2016 Corinna Vinschen <corinna@vinschen.de> 
Modified (m) 2017 Thomas Wolff: revise Unicode and locale/wchar handling
 */
#define _DEFAULT_SOURCE
#include <ctype.h>

#undef iscntrl_l
int
iscntrl_l (int c, struct __locale_t *locale)
{
    (void) locale;
#if _PICOLIBC_CTYPE_SMALL
    return (0x00 <= c && c <= 0x1f) || c == 0x7f;
#else
    return __locale_ctype_ptr_l (locale)[c+1] & _C;
#endif
}
