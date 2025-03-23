/* Routine to translate between Japanese characters and Unicode */

/* Copyright (c) 2002 Red Hat Incorporated.
   All rights reserved.
   Modified (m) 2017 Thomas Wolff: consider locale, add dummy uc2jp

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

     Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

     Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.

     The name of Red Hat Incorporated may not be used to endorse
     or promote products derived from this software without specific
     prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED.  IN NO EVENT SHALL RED HAT INCORPORATED BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <picolibc.h>

#include <string.h>
#include <wctype.h>
#include "local.h"

/* Japanese to Unicode conversion routine */
#ifdef __MB_EXTENDED_CHARSETS_JIS

#include "jp2uc.h"

wint_t
__jp2uc (wint_t c, int type)
{
  unsigned index;
  unsigned char byte1, byte2, sjis1, sjis2;
  wint_t ret;

  /* we actually use tables of EUCJP to Unicode.  For JIS, we simply
     note that EUCJP is essentially JIS with the top bits on in each
     byte and translate to EUCJP.  For SJIS, we do a translation to EUCJP before
     accessing the tables. */
  switch (type)
    {
    case JP_JIS:
      byte1 = (c >> 8) + 0x80;
      byte2 = (c & 0xff) + 0x80;
      break;
    case JP_EUCJP:
      byte1 = (c >> 8);
      byte2 = (c & 0xff);
      if (byte1 == 0)
        {
          switch (byte2) {
          case 0x8e:
          case 0x8f:
          case 0xa0:
          case 0xff:
            break;
          default:
            return byte2;
          }
        }
      break;
    case JP_SJIS:
      sjis1 = c >> 8;
      sjis2 = c & 0xff;
      if (sjis1 == 0)
        {
          switch(sjis2) {
          case 0x5c:
            return 0xa5;        /* ¥ */
          case 0x7e:
            return 0x203e;      /* ‾ */
          default:
            if (0xa1 <= sjis2 && sjis2 <= 0xdf)
              return sjis2 + (0xff61 - 0xa1);
            return sjis2;
          }
        }

      if (0x81 <= sjis1 && sjis1 <= 0x9f)
          byte1 = (sjis1 - 112 + 64) << 1;
      else if (0xe0 <= sjis1 && sjis1 <= 0xef)
          byte1 = (sjis1 - 176 + 64) << 1;
      else
          return WEOF;

      if (0x40 <= sjis2 && sjis2 <= 0x9e && sjis2 != 0x7f) {
          byte1 -= 1;
          byte2 = sjis2 - 31 + 128;
          if (sjis2 >= 0x7f)
              byte2--;
      } else if (0x9f <= sjis2 && sjis2 <= 0xfc) {
          byte2 = sjis2 - 126 + 128;
      } else {
          return WEOF;
      }
      break;
    default:
      return WEOF;
    }

#define in_bounds(array, val)   (val < (sizeof(array)/sizeof(array[0])))

#define in_bounds_o(array, val, offset)   (offset <= val && val < offset + (sizeof(array)/sizeof(array[0])))

  /* find conversion in jp2uc arrays */

  /* handle larger ranges first */
  if (byte1 >= 0xb0 && byte1 <= 0xcf && c <= (wint_t) 0xcfd3 && byte2 >= 0xa1)
    {
      index = (byte1 - 0xb0) * 94 + (byte2 - 0xa1);
      if (in_bounds(b02cf, index))
        return b02cf[index];
    }
  else if (byte1 >= 0xd0 && byte1 <= 0xf4 && c <= (wint_t) 0xf4a6 && byte2 >= 0xa1)
    {
      index = (byte1 - 0xd0) * 94 + (byte2 - 0xa1);
      if (in_bounds(d02f4, index))
        return d02f4[index];
    }

  /* handle smaller ranges here */
  switch (byte1)
    {
    case 0x8E:
        /* Upper half of JIS X 0201 */
        if (0xa1 <= byte2 && byte2 <= 0xdf)
            return byte2 + (0xff61 - 0xa1);
        break;
    case 0xA1:
      if (!in_bounds_o (a1, byte2, 0xa1))
        break;
      return (wint_t)a1[byte2 - 0xa1];
    case 0xA2:
      if (!in_bounds_o (a2, byte2, 0xa1))
        break;
      ret = a2[byte2 - 0xa1];
      if (ret != 0)
	return (wint_t)ret;
      break;
    case 0xA3:
      if (!in_bounds_o (a3, byte2, 0xa1))
        break;
      if (a3[byte2 - 0xa1])
	return (wint_t)(0xff00 + (byte2 - 0xa0));
      break;
    case 0xA4:
      if (byte2 <= 0xf3)
	return (wint_t)(0x3000 + (byte2 - 0x60));
      break;
    case 0xA5:
      if (byte2 <= 0xf6)
	return (wint_t)(0x3000 + byte2);
      break;
    case 0xA6:
      if (!in_bounds_o (a6, byte2, 0xa1))
        break;
      ret = 0;
      if (byte2 <= 0xd8)
	ret = (wint_t)a6[byte2 - 0xa1];
      if (ret != 0)
	return ret;
      break;
    case 0xA7:
      if (!in_bounds_o (a7, byte2, 0xa1))
        break;
      ret = 0;
      if (byte2 <= 0xf1)
	ret = (wint_t)a7[byte2 - 0xa1];
      if (ret != 0)
	return ret;
      break;
    case 0xA8:
      if (!in_bounds_o (a8, byte2, 0xa1))
        break;
      if (byte2 <= 0xc0)
	return (wint_t)a8[byte2 - 0xa1];
      break;
    default:
      break;
    }

  return WEOF;
}

/* Unicode to Japanese conversion routine. Not the most elegant solution. */
wint_t
__uc2jp (wint_t c, int type)
{
  wint_t u;
  if (c == WEOF)
    return WEOF;
  for (u = 0x0000; ; u++) {
    if (__jp2uc(u, type) == c)
      return u;
    if (u == 0xffff)
      return WEOF;
  }
}

#endif /* __MB_EXTENDED_CHARSETS_JIS */
