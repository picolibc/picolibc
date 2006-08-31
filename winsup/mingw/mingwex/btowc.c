#include "mb_wc_common.h"
#include <wchar.h>
#include <stdio.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

wint_t btowc (int c)
{
  if (c == EOF)
    return (WEOF);
  else
    {
      unsigned char ch = c;
      wchar_t wc = WEOF;
      MultiByteToWideChar (get_codepage(), MB_ERR_INVALID_CHARS,
			   (char*)&ch, 1, &wc, 1);
      return wc;
    }
}
