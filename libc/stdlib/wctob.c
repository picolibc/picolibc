/*
Copyright (c) 2002 Thomas Fitzsimmons <fitzsim@redhat.com>
 */
#include <wchar.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include "local.h"

int
wctob(wint_t wc)
{
    mbstate_t     mbs;
    unsigned char pmb[MB_LEN_MAX];
    uintmax_t     uwc = (uintmax_t)wc;

    /* The unsigned conversion maps negative values above WCHAR_MAX. */
    if (wc == WEOF || uwc > (uintmax_t)WCHAR_MAX)
        return EOF;

    /* Put mbs in initial state. */
    memset(&mbs, '\0', sizeof(mbs));

    return __WCTOMB((char *)pmb, wc, &mbs) == 1 ? (int)pmb[0] : EOF;
}
