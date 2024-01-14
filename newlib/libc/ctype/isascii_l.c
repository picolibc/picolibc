/*
Copyright (c) 2016 Corinna Vinschen <corinna@vinschen.de> 
Modified (m) 2017 Thomas Wolff: revise Unicode and locale/wchar handling
 */
#define _DEFAULT_SOURCE
#include <ctype.h>

#undef isascii_l
int
isascii_l (int c, struct __locale_t *locale)
{
    (void) locale;
    return (unsigned) c <= 0177;
}
