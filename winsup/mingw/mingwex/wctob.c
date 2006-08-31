#include "mb_wc_common.h"
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* Return just the first byte after translating to multibyte.  */
int wctob (wint_t wc )
{
    wchar_t w = wc;
    char c;
    int invalid_char = 0;
    if (!WideCharToMultiByte (get_codepage(), 
			      0 /* Is this correct flag? */,
			      &w, 1, &c, 1, NULL, &invalid_char)
         || invalid_char)
      return EOF;
    return (int) c;
}
